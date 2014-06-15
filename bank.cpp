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

#include "bank.h"

namespace membles
{

/* ctor: Bank
 * Initalize everything to zero
 */
Bank::Bank()
    : state_(IDLE),
      open_row_(0),
      in_use_(false),
      next_rd_(0),
      next_wr_(0),
      next_act_(0),
      next_pre_(0),
      next_pd_(0),
      next_pu_(0),
      countdown_(0)
{}


/*
 * move 1 cycle ahread
 */
void Bank::step()
{
    if (countdown_) {
        countdown_--;
        if (countdown_ == 0) {
            if (state_ == ACTIVATING) {
                state_ = ACTIVE;
            } else if (state_ == PRECHARGING) {
                state_ = IDLE;
            } else if (state_ == REFRESHING) {
                state_ = IDLE;
            } else {
                ERROR("Counting down during a non-intermediate state");
            }
        }
    }
    cycle_++;
}


/*
 * Process activate operations
 * Banks not being accessed but attached to the same rank also need to adjust
 *   their status
 */
void Bank::activate(uint32_t row, bool this_bank, bool this_rank)
{
    // change bank state
    if (this_bank) {
        assert(state_ == IDLE);
        state_ = ACTIVATING;
        open_row_ = row;
        countdown_ = dev_cfg_->tRCD();
    }
    // change timing
    if (this_bank) {
        next_rd_ = max(next_rd_, cycle_ + dev_cfg_->tRCD() - dev_cfg_->AL);
        next_wr_ = max(next_rd_, cycle_ + dev_cfg_->tRCD() - dev_cfg_->AL);
        next_act_ = max(next_act_, cycle_ + dev_cfg_->tRC());
        next_pre_ = max(next_pre_, cycle_ + dev_cfg_->tRAS());
    } else if (this_rank) {
        next_act_ = max(next_act_, cycle_ + dev_cfg_->tRRD());
    }
}


/*
 * Process precharge operations
 * Banks not being accessed but attached to the same rank also need to adjust
 *   their status
 */
void Bank::precharge(bool this_bank, bool this_rank)
{
    // change bank state
    if (this_bank) {
        assert(state_ == ACTIVE);
        state_ = PRECHARGING;
        countdown_ = dev_cfg_->tRP();
    }
    // change timing
    if (this_bank) {
        next_act_ = max(next_act_, cycle_ + dev_cfg_->tRP());
        next_rd_ = max(next_rd_, next_act_ + dev_cfg_->tRCD());
        next_wr_ = max(next_wr_, next_act_ + dev_cfg_->tRCD());
        next_pre_ = max(next_pre_, next_act_ + dev_cfg_->tRAS());
    }
}


/*
 * Process read operations
 * Banks not being accessed but attached to the same rank also need to adjust
 *   their status
 */
void Bank::read(bool this_bank, bool this_rank)
{
    // check the bank state
    if (this_bank) {
        assert(state_ == ACTIVE);
    }
    // change timing
    if (this_rank) {
        next_rd_ = max(next_rd_, cycle_ + dev_cfg_->tCCD());
    } else {
        next_rd_ = max(next_rd_, cycle_ + dev_cfg_->BL / dev_cfg_->data_rate_
                       + 1); // TODO
    }
    next_wr_ = max(next_wr_, cycle_ + dev_cfg_->RdToWr());
    if (this_bank) {
        next_pre_ = max(next_pre_, cycle_ + dev_cfg_->RdToPre());
        next_act_ = max(next_act_, next_pre_ + dev_cfg_->tRP());
    }
    next_pd_ = max(next_pd_, cycle_); // TODO
    next_pu_ = max(next_pu_, cycle_); // TODO
}


/*
 * Process write operations
 * Banks not being accessed but attached to the same rank also need to adjust
 *   their status
 */
void Bank::write(bool this_bank, bool this_rank)
{
    // check the bank state
    if (this_bank) {
        assert(state_ == ACTIVE);
    }
    // change timing
    next_rd_ = max(next_rd_, cycle_ + dev_cfg_->WrToRd(this_rank));
    if (this_rank) {
        next_wr_ = max(next_wr_, cycle_ + dev_cfg_->tCCD());
    } else {
        next_wr_ = max(next_wr_, cycle_ + dev_cfg_->BL / dev_cfg_->data_rate_
                       + 1);
    }
    if (this_bank) {
        next_pre_ = max(next_pre_, cycle_ + dev_cfg_->WrToPre());
        next_act_ = max(next_act_, next_pre_ + dev_cfg_->tRP());
    }
    next_pd_ = max(next_pd_, cycle_); // TODO
    next_pu_ = max(next_pu_, cycle_); // TODO
}


/*
 * Process a command
 * Just a wrapper
 */
void Bank::operate(Command *cmd, bool this_bank, bool this_rank)
{
    CmdType type = cmd->type();
    if (type == ACTIVATE) {
        activate(cmd->row(), this_bank, this_rank);
    } else if (type == PRECHARGE) {
        precharge(this_bank, this_rank);
    } else if (type == READ) {
        read(this_bank, this_rank);
    } else if (type == WRITE) {
        write(this_bank, this_rank);
    } else {
        // TODO
    }
}


/*
 * Return the next cycle according to command type
 * e.g. READ -> return next_rd
 */
Cycle Bank::next(Command *cmd)
{
    switch (cmd->type()) {
    case READ:
        return (state_ == ACTIVE && open_row_ == cmd->row()) ? next_rd_ :
                MAX_CYCLE;
    case WRITE:
        return (state_ == ACTIVE && open_row_ == cmd->row()) ? next_wr_ :
                MAX_CYCLE;
    case ACTIVATE:
        return state_ == IDLE ? next_act_ : MAX_CYCLE;
    case PRECHARGE:
        return state_ == ACTIVE ? next_pre_ : MAX_CYCLE;
    default:
        // TODO: lot of others
        return MAX_CYCLE;
    }
}


/*
 * Return the earliest cycle that issuing a transaction becomes possible
 */
Cycle Bank::EarliestCycle(uint32_t row, bool is_read)
{
    if (in_use_) {
        // this bank is being used by other transaction
        return MAX_CYCLE;
    } else if (state_ == ACTIVE) {
        // this bank is open, determine whether it's page hit or conflict
        if (open_row_ == row) {
            // page hit
            return next_rd_;
        } else {
            // page conflict
            return next_act_ + dev_cfg_->tRCD();
        }
    } else if (state_ == IDLE) {
        // this bank is close, it's page miss
        return next_act_ + dev_cfg_->tRCD();
    } else {
        // TODO: further model power-down, self-refresh
        return cycle_;
    }
}

}
