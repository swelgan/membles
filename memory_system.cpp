/* Copyright (c) 2014, Jue Wang
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "memory_system.h"

namespace membles
{

/* ctor: Memory System
 * The number of channels is 1 by default
 */
MemorySystem::MemorySystem()
    : BaseObj(),
      num_chan_(1),
      chan_itlv_bit_(10)
{}


/* dtor: Memory System
 * deallocate output stream if necesary
 */
MemorySystem::~MemorySystem()
{
    if (log_) {
        log_->close();
        delete log_;
    }
    if (csv_) {
        csv_->close();
        delete csv_;
    }
    if (trc_) {
        trc_->close();
        delete trc_;
    }
}


/*
 * Initialize the memory system by specifying:
 *   the controller configuration including the number of channels
 *   an array of device configuration and channel capacity
 *   If the array size is less than channel count, then assuming a homogeneous
 *   channel seeting
 */
bool MemorySystem::init(const string &ctrl_filename,
                        const vector<string> &dev_filenames,
                        vector<uint64_t> sizes)
{
    // TODO: just for test
    trc_ = new ofstream;
    trc_->open("test.trc", ios_base::out | ios_base::trunc);

    sizes_ = sizes;
    bool success = true;
    // load controller configuration file
    success &= ctrl_cfg_.ReadFile(ctrl_filename);
    num_chan_ = ctrl_cfg_.num_chan;
    chan_itlv_bit_ = ctrl_cfg_.chan_itlv_bit;
    freq_ = ctrl_cfg_.ctrl_freq;
    // load device configuration files
    if (num_chan_ < dev_filenames.size()) {
        ERROR("User provides " << dev_filenames.size() << " device "
              "configurations in a " << num_chan_  << "-channel system");
        return false;
    }
    if (sizes.size() != 1 && num_chan_ != sizes.size()) {
        ERROR("User provides " << sizes.size() << " capacity settings "
              "in a " << num_chan_  << "-channel system");
        return false;
    }
    dev_cfgs_.resize(num_chan_);
    // set verbosity
    if (verbose_) {
        for (auto &cfg : dev_cfgs_) cfg.set_verbose();
    }
    // load device configuration one by one
    for (size_t i = 0; i < num_chan_; ++i) {
        size_t index = i;
        if (index >= dev_filenames.size()) index = dev_filenames.size() - 1;
        success &= dev_cfgs_[i].ReadFile(dev_filenames[index]);
    }
    // set channel capacity
    sizes_.resize(num_chan_);
    for (size_t i = 0; i < num_chan_; ++i) {
        if (sizes.size() == 1) {
            sizes_[i] = sizes[0] / num_chan_;
        } else {
            sizes_[i] = sizes[i];
        }
        success &= dev_cfgs_[i].derive(sizes_[i], ctrl_cfg_);
    }
    // check if channel capacit can be correctly partitioned
    if (sizes.size() == 1 && sizes_[0] * num_chan_ != sizes[0]) {
        ERROR(sizes[0] << "MB cannot be evenly divided into a " << num_chan_
              << "-channel system");
        return false;
    }
    // check if the entire initialization is successful
    if (!success) {
        ERROR("Fail to initialize the memory system.");
        return false;
    }

    // create components
    // create N channels depending on the input setting
    channels_.resize(num_chan_);
    for (size_t i = 0; i < num_chan_; ++i) {
        if (verbose_) channels_[i].set_verbose();
        success &= channels_[i].init(i, &ctrl_cfg_, &(dev_cfgs_[i]), log_,
                                     csv_, trc_);
    }

    return success;
}


/*
 * Simulate one cycle ahead
 */
void MemorySystem::step()
{
    // TODO
    cycle_++;

    for (size_t i = 0; i < num_chan_; ++i) {
        channels_[i].step();
    }
}


/*
 * Find which channel a transaciton belongs to
 */
uint32_t MemorySystem::FindChanId(Transaction *tx)
{
    assert(tx);
    if (num_chan_ == 1) return 0;
    uint64_t mask = (1UL << num_chan_) - 1;
    return (tx->addr() >> chan_itlv_bit_) & mask;
}  


/*
 * Add a transaction into the memory system
 * Return false if the transaction queue cannot hold the incoming transaction
 */
bool MemorySystem::AddTx(Transaction *tx)
{
    if (tx->len() > (1UL << chan_itlv_bit_)) {
        ERROR("Found a transaction whose length is larger than channel "
              "interleaving granularity");
        return false;
    }
    uint32_t chan = FindChanId(tx);
    return channels_[chan].AddTx(tx);
}


/*
 * Print some statistics of the memory system
 */
void MemorySystem::stat()
{
    // TODO
}


/*
 * Override this function because we need to cascade the setting
 */
void MemorySystem::set_verbose()
{
    BaseObj::set_verbose();
    // set verbosity to controller configuration
    // device configuration verbosity will be correctly set later
    ctrl_cfg_.set_verbose();
    // TODO
}

}
