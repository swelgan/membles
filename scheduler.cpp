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

#include "scheduler.h"
#include "channel.h"

namespace membles
{

/* ctor: Scheduler
 * Initialize address mapper and bank state table references
 */
Scheduler::Scheduler(Channel *parent, AddressMap &mapper,
                     vector<vector<Bank>> &banks)
    : parent_(parent),
      mapper_(mapper),
      banks_(banks)
{}


/*
 * Initialize the scheduler
 * Resize the bank state table size
 */
bool Scheduler::init(CtrlCfg *ctrl_cfg, DevCfg *dev_cfg,
                     ofstream *log, ofstream *csv, ofstream *trc)
{
    bool success = MemObj::init(ctrl_cfg, dev_cfg, log, csv, trc);
    if (!success) return false;
    
    return success;
}


/*
 * move 1 cycle forward
 */
void Scheduler::step()
{
    Command *cmd = schedule();
    // if a command is ready to execute
    if (cmd) {
        if (verbose_) INFO("@" << cycle_ << ": Command issued: " << *cmd);
        if (trc_) {
            *trc_ << "CH" << parent_->id() << " " << cycle_ << " ";
            switch (cmd->type()) {
            case READ:
                *trc_ << "READ";
                break;
            case WRITE:
                *trc_ << "WRITE";
                break;
            case ACTIVATE:
                *trc_ << "ROWACT";
                break;
            case PRECHARGE:
                *trc_ << "PRECHARGE";
                break;
            default:
                *trc_ << "UNKNOWN";
            }
            *trc_ << " " << cmd->tx()->id() << " " << cmd->rank() << " "
                << cmd->bank() << " " << cmd->row() << " " << cmd->col()
                << endl;
        }

        execute(cmd);
        for (auto iter = cmd_queue_.begin(); iter != cmd_queue_.end(); ++iter) {
            if (*iter == cmd) {
                cmd_queue_.erase(iter);
                break;
            }
        }
    }

    cycle_++;
}


/*
 * Break a transaction into bus commands and add them into command queue
 * Return false if command queue lacks of space
 * need_act indicates if it is a page hit or not
 * need_pre further indicates if it is a page miss or page conflict
 */
bool Scheduler::AddTx(Transaction *tx, bool need_act, bool need_pre)
{
    uint64_t addr = tx->addr();
    uint32_t len = tx->len();
    uint16_t priority = tx->priority();
    uint32_t mal = dev_cfg_->mal;
    // MAL size alignment
    align(addr, len, mal);
    // TODO: only support MAL-sized transaciton now
    assert(len == mal);

    // check if command queue has enough space
    size_t to_fill = 1;
    // consider if ACT is needed
    if (need_act) to_fill++;
    // consider if PRE is needed
    if (need_pre) to_fill++;
    // double the size because we reserve an ACTIVATE for each READ/WRITE
    to_fill *= 2;
    if (cmd_queue_.size() + to_fill > max_cmd_queue_depth_)
        return false;
    
    // decoding address
    uint32_t chan, rank, bank, row, col;
    mapper_.map(addr, chan, rank, bank, row, col);

    if (need_pre) {
        // generate PERCHARGE command
        PreCmd *pre = new PreCmd(cycle_, chan, rank, bank, priority, tx);
        if (verbose_) INFO("@" << cycle_ << ": Command added: " << *pre);
        cmd_queue_.insert(pre);
    }

    if (need_act) {
        // generate ACTIVATE command
        ActCmd *act = new ActCmd(cycle_, chan, rank, bank, row,
                                 priority, tx);
        if (verbose_) INFO("@" << cycle_ << ": Command added: " << *act);
        cmd_queue_.insert(act);
    }

    // generate READ/WRITE command
    if (tx->is_read()) {
        ReadCmd *rd = new ReadCmd(cycle_, chan, rank, bank, row, col,
                                  priority, tx);
        if (verbose_) INFO("@" << cycle_ << ": Command added: " << *rd);
        cmd_queue_.insert(rd);
    } else {
        WriteCmd *wr = new WriteCmd(cycle_, chan, rank, bank, row, col,
                                    priority, tx);
        if (verbose_) INFO("Command added: " << *wr);
        cmd_queue_.insert(wr);
    }

    if (verbose_) {
        INFO("Transaction " << tx->id() << " successfully scheduled");
    }

    return true;
}


/*
 * Schedule the next bus command
 */
Command *Scheduler::schedule()
{
    // TODO consider open-page only
    for (auto iter = cmd_queue_.begin(); iter != cmd_queue_.end(); ++iter) {
        // the order is already maintained by STL set data structure
        // traverse the command queue from begin to end
        // return the first issuable command, and calculate the next scheduling
        //   cycle
        Command *this_cmd = *iter;
        uint32_t rank = this_cmd->rank();
        uint32_t bank = this_cmd->bank();
        Bank &b = banks_[rank][bank];
        if (b.next(this_cmd) <= cycle_) return this_cmd;
    }

    return nullptr;
}


/*
 * Execute a command, make impact to its associated banks
 */
void Scheduler::execute(Command *cmd)
{
    uint32_t num_rank = dev_cfg_->num_rank;
    uint32_t num_bank = dev_cfg_->num_bank;
    uint32_t rank = cmd->rank();
    uint32_t bank = cmd->bank();
    // all the banks on the same channel (different or same ranks) might be
    //   impacted by this command
    for (uint32_t r = 0; r < num_rank; ++r) {
        for (uint32_t b = 0; b < num_bank; ++b) {
            if (r != rank) {
                // different rank
                banks_[r][b].operate(cmd, false, false);
            } else {
                // same rank
                if (b != bank) {
                    // different bank
                    banks_[r][b].operate(cmd, false, true);
                } else {
                    // same bank
                    banks_[r][b].operate(cmd, true, true);
                    // release bank if work is done
                    if (cmd->type() == READ || cmd->type() == WRITE) {
                        parent_->process(cmd);
                    }
                }
            }
        }
    }
}


/*
 * Set max command queue depth
 */
void Scheduler::SetCmdQueueDepth(uint32_t max_cmd_queue_depth)
{
    max_cmd_queue_depth_ = max_cmd_queue_depth;
}

}
