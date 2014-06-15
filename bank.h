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

#ifndef BANK_H
#define BANK_H

#include "macro.h"
#include "base_obj.h"
#include "command.h"

namespace membles
{

// different states in memory state machine
enum BankState {
    IDLE,
    ACTIVATING,
    ACTIVE,
    PRECHARGING,
    REFRESHING,
    POWER_DOWN,
    DEEP_POWER_DOWN,
    SELF_REFRESHING
};


class Bank : public MemObj
{

  public:

    Bank();

    void step();

    // accessors
    BankState state() const { return state_; }
    uint32_t open_row() const { return open_row_; }
    bool in_use() const { return in_use_; }

    void use() { in_use_ = true; }
    void release() { in_use_ = false; }
    void activate(uint32_t row, bool this_bank = true, bool this_rank = true);
    void precharge(bool this_bank = true, bool this_rank = true);
    void read(bool this_bank = true, bool this_rank = true);
    void write(bool this_bank = true, bool this_rank = true);
    void operate(Command *cmd, bool this_bank = true, bool this_rank = true);

    Cycle next(Command *cmd);
    Cycle EarliestCycle(uint32_t row, bool is_read);

  private:

    BankState state_;

    // indicate which row is opened in this bank
    uint32_t open_row_;

    // indicate whether this bank is being used for a transaction
    bool in_use_;

    // the earliest cycle that a command is allowed
    Cycle next_rd_;     // read
    Cycle next_wr_;     // normal write
    //Cycle next_mwr_;    // mask write
    Cycle next_act_;    // activate
    Cycle next_pre_;    // precharge
    Cycle next_pd_;     // power-down
    Cycle next_pu_;     // exit power-down

    // if the state machine is going to switch to another state automatically,
    //   we set a countdown here
    Cycle countdown_;

};

}

#endif
