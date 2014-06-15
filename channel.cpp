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

#include "channel.h"

namespace membles
{

/* ctor: Channel
 * Pass address mapper and bank table references to scheduler
 */
Channel::Channel()
    : sched_(this, mapper_, banks_)
{}


/* dtor: Channel
 * Delete any remaining transactions in the read and the write queues
 */
Channel::~Channel()
{
    while (!rd_queue_.empty()) {
        if (rd_queue_.back()) delete rd_queue_.back();
        rd_queue_.pop_back();
    }
    while (!wr_queue_.empty()) {
        if (wr_queue_.back()) delete wr_queue_.back();
        wr_queue_.pop_back();
    }
}


/*
 * Move 1 cycle forward
 */
void Channel::step()
{
    sched_.step();
    for (auto &b1 : banks_) {
        for (auto &b2 : b1) {
            b2.step();
        }
    }
    //INFO("rd: " << rd_queue_.size() << "+" << rd_resp_queue_.size());

    // TODO
    cycle_++;

    DispatchTransaction();
}


/*
 * Initialize the channel by specifying:
 *   a controller configuration
 *   a device fconfiguration
 *   a set of output stream
 */
bool Channel::init(uint16_t id, CtrlCfg *ctrl_cfg, DevCfg *dev_cfg,
                   ofstream *log, ofstream *csv, ofstream *trc)
{
    id_ = id;
    bool success = MemObj::init(ctrl_cfg, dev_cfg, log, csv, trc);

    success &= mapper_.init(ctrl_cfg_, dev_cfg_);
    success &= sched_.init(ctrl_cfg_, dev_cfg_, log_, csv_, trc_);
    
    if (!success) return false;
    
    // get number of ranks and banks
    uint32_t num_rank = dev_cfg_->num_rank;
    uint32_t num_bank = dev_cfg_->num_bank;
    // resize bank state table
    banks_.resize(num_rank);
    for (auto &b1 : banks_) {
        b1.resize(num_bank);
        for (auto &b2 : b1) {
            success &= b2.init(ctrl_cfg, dev_cfg, log, csv, trc);
            if (verbose_) b2.set_verbose();
        }
    }

    if (!success) return false;
    
    // forward related parameters
    sched_.SetCmdQueueDepth(ctrl_cfg_->max_cmd_queue_depth);

    // set verbosity
    if (verbose_) {
        sched_.set_verbose();
        INFO("Channel " << id_ << " address mapping:");
        mapper_.info();
    }

    return success;
}


/*
 * Add a transaction into a proper queue
 * Return false if there is no enough room in the queue
 */
bool Channel::AddTx(Transaction *tx)
{
    if (tx->is_read()) {
        // add to read queue
        if (rd_queue_.size() + rd_resp_queue_.size() <
                ctrl_cfg_->max_rd_queue_depth) {
            rd_queue_.push_back(tx);
            return true;
        } else {
            return false;
        }
    } else {
        // add to write queue
        if (wr_queue_.size() + wr_resp_queue_.size() <
                ctrl_cfg_->max_wr_queue_depth) {
            wr_queue_.push_back(tx);
            // turn on write draining if write buffer is full
            wr_draining_ |= (wr_queue_.size() == ctrl_cfg_->max_wr_queue_depth);
            return true;
        } else {
            return false;
        }
    }
}


/*
 * Print some statistics of the channel
 */
void Channel::stat()
{
    // TODO
}


/*
 * Dispatch transactions into scheduler
 * Read queue has priority over write transaction
 * Write only wins when the channel is in the write draining state
 * Return false if nothing has been dispatched
 */
bool Channel::DispatchTransaction()
{
    // do nothing it read or write transaction is empty
    if (rd_queue_.empty() && wr_queue_.empty())
        return false;

    // dispatch read transaction if something in the read queue and we are not
    //   in the write draining state
    if (!rd_queue_.empty() && !wr_draining_) {
        return DispatchRead();
    } else {
        return DispatchWrite();
    }
}


/*
 * A helper function to dispatch a transaciton from read queue
 * The dispatch order varies depending on scheduling algorithms
 */
bool Channel::DispatchRead()
{
    // the read transaciton queue should have something
    assert(!rd_queue_.empty());

    // find the candidate transaction based on scheudling algorithm
    // TODO: only support FR-FCFS at this time
    Transaction *selected = nullptr;
    auto selected_iter = rd_queue_.begin();
    Cycle issue_cycle = MAX_CYCLE;
    Bank *target_bank = nullptr;
    uint32_t target_row = 0;
    for (auto iter = rd_queue_.begin(); iter != rd_queue_.end(); ++iter) {
        Transaction *this_tx = *iter;
        // map this transaction to DRAM channel, rank, bank, row, column
        uint64_t addr = this_tx->addr();
        // TODO: only support MAL-sized transaction at this time
        assert(this_tx->len() == dev_cfg_->mal);
        uint32_t chan, rank, bank, row, col;
        mapper_.map(addr, chan, rank, bank, row, col);
        // check if channel mapping is correct
        assert(chan == id_);
        // select the target bank
        Bank &b = banks_[rank][bank];
        // find the transaciton that has the earliest issue cycle
        Cycle this_issue_cycle = b.EarliestCycle(row, this_tx->is_read());
        if (this_issue_cycle < issue_cycle) {
            issue_cycle = this_issue_cycle;
            selected = this_tx;
            selected_iter = iter;
            target_bank = &(banks_[rank][bank]);
            target_row = row;
        }
    }

    if (selected == nullptr) {
        // nothing can be issued
        return false;
    }

    assert(target_bank);
    bool success = true;
    if (target_bank->state() == ACTIVE) {
        if (target_bank->open_row() == target_row) {
            // page hit, need no ACt, need no PRE
            success = sched_.AddTx(selected, false, false);
        } else {
            // page conflict, need ACT, need PRE
            success = sched_.AddTx(selected, true, true);
        }
    } else {
        // page miss, need ACT, need no PRE
        success = sched_.AddTx(selected, true, false);
    }

    if (success) {       
        // successfully scheduled this read transaction
        // move it to response queue
        rd_resp_queue_.push_back(selected);
        rd_queue_.erase(selected_iter);
        // mark bank in use
        target_bank->use();
    } else {
        // scheduler does not have enough command queue space
        return false;
    }

    // turn on write draining if read queue is empty but write queue is not
    if (rd_queue_.empty() && !wr_queue_.empty()) wr_draining_ = true;

    return true;
}


/*
 * A helper function to dispatch a transaction from write queue
 */
bool Channel::DispatchWrite()
{
    // the write transaction queue should have something
    assert(!wr_queue_.empty());

    // find the candidate transaction based on scheudling algorithm
    // TODO: only support FR-FCFS at this time
    Transaction *selected = nullptr;
    auto selected_iter = wr_queue_.begin();
    Cycle issue_cycle = MAX_CYCLE;
    Bank *target_bank = nullptr;
    uint32_t target_row = 0;
    for (auto iter = wr_queue_.begin(); iter != wr_queue_.end(); ++iter) {
        Transaction *this_tx = *iter;
        // map this transaction to DRAM channel, rank, bank, row, column
        uint64_t addr = this_tx->addr();
        // TODO: only support MAL-sized transaction at this time
        assert(this_tx->len() == dev_cfg_->mal);
        uint32_t chan, rank, bank, row, col;
        mapper_.map(addr, chan, rank, bank, row, col);
        // check if channel mapping is correct
        assert(chan == id_);
        // select the target bank
        Bank &b = banks_[rank][bank];
        // find the transaciton that has the earliest issue cycle
        Cycle this_issue_cycle = b.EarliestCycle(row, this_tx->is_read());
        if (this_issue_cycle < issue_cycle) {
            issue_cycle = this_issue_cycle;
            selected = this_tx;
            selected_iter = iter;
            target_bank = &(banks_[rank][bank]);
            target_row = row;
        }
    }

    if (selected == nullptr) {
        // nothing can be issued
        return false;
    }

    assert(target_bank);
    bool success = true;
    if (target_bank->state() == ACTIVE) {
        if (target_bank->open_row() == target_row) {
            // page hit, need no ACt, need no PRE
            success = sched_.AddTx(selected, false, false);
        } else {
            // page conflict, need ACT, need PRE
            success = sched_.AddTx(selected, true, true);
        }
    } else {
        // page miss, need ACT, need no PRE
        success = sched_.AddTx(selected, true, false);
    }

    if (success) {
        // succesfully scheduled this write transaction
        // move it to response queue
        wr_resp_queue_.push_back(selected);
        wr_queue_.erase(selected_iter);
        // mark bank in use
        target_bank->use();
        // TODO write compelete call
    } else {
        // scheduler does not have enough space
        return false;
    }

    // turn off write draining if write queue is fully drained
    if (wr_queue_.empty()) wr_draining_ = false;

    return true;
}


/*
 * A transaciton has been processed by scheduler.
 * Now we need to handle the response queue.
 */
void Channel::process(Command *cmd)
{
    Transaction *tx = cmd->tx();
    // this transaction must be in a response queue
    if (tx->is_read()) {
        auto iter = rd_resp_queue_.begin();
        for (; iter != rd_resp_queue_.end(); ++iter) {
            if (*iter == tx) break;
        }
        // we must find something
        assert(iter != rd_resp_queue_.end());
        rd_resp_queue_.erase(iter);
    } else {
        auto iter = wr_resp_queue_.begin();
        for (; iter != wr_resp_queue_.end(); ++iter) {
            if (*iter == tx) break;
        }
        // we must find something
        assert(iter != wr_resp_queue_.end());
        wr_resp_queue_.erase(iter);
    }
    // release the in-use bank
    banks_[cmd->rank()][cmd->bank()].release();

    // TODO: notify upper level caller
}

}
