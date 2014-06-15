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

#ifndef COMMAND_H
#define COMMAND_H

#include "macro.h"
#include "transaction.h"

namespace membles
{

/*
 * Memory bus command type
 */
enum CmdType {
    READ,
    WRITE,
    READ_AP,        // read with auto prechrage
    WRITE_AP,       // write with auto precharge
    ACTIVATE,
    PRECHARGE,      // per-bank precharge
    PRECHARGE_AB,   // all-bank precharge
    REFRESH,        // all-bank refresh
    REFRESH_PB,     // per-bank refresh
    ENTER_SELF_REFRESH,
    ENTER_DEEP_PD,  // enter deep power down
    ENTER_PD,       // enter power down
    EXIT_PD         // exit deep/normal power down or self refresh
};


class Command
{
  
  public:

    Command(Cycle birth_cycle);

    // accessors
    uint64_t id() const { return id_; }
    Cycle birth_cycle() const { return birth_cycle_; }
    CmdType type() const { return type_; }
    uint32_t chan() const { return chan_; }
    uint32_t rank() const { return rank_; }
    uint32_t bank() const { return bank_; }
    uint32_t row() const { return row_; }
    uint32_t col() const { return col_; }
    uint16_t priority() const { return priority_; }
    Transaction *tx() const { return parent_tx_; }

    friend ostream &operator<<(ostream &os, const Command &cmd);

  protected:

    // command ID
    uint64_t id_;

    // birth cycle
    Cycle birth_cycle_;

    // command type
    CmdType type_;

    // addresses
    uint32_t chan_;
    uint32_t rank_;
    uint32_t bank_;
    uint32_t row_;
    uint32_t col_;

    // priority
    uint16_t priority_;

    // a back pointer to its associated transaction
    Transaction *parent_tx_;

  private:

    static uint64_t count;

};


/*
 * just a wrapper for ACTIVATE commands
 */
class ActCmd : public Command
{

  public:

    ActCmd(Cycle birth_cycle, uint32_t chan, uint32_t rank, uint32_t bank,
           uint32_t row, uint16_t priority = 0, Transaction *tx = nullptr)
        : Command(birth_cycle)
    {
        type_ = ACTIVATE;
        chan_ = chan;
        rank_ = rank;
        bank_ = bank;
        row_ = row;
        priority_ = priority;
        parent_tx_ = tx;
    }

};


/*
 * just a wrapper for PRECHARGE commands
 */
class PreCmd : public Command
{

  public:

    PreCmd(Cycle birth_cycle, uint32_t chan, uint32_t rank, uint32_t bank,
           uint16_t priority = 0, Transaction *tx = nullptr)
        : Command(birth_cycle)
    {
        type_ = PRECHARGE;
        chan_ = chan;
        rank_ = rank;
        bank_ = bank;
        priority_ = priority;
        parent_tx_ = tx;
    }

};


/*
 * just a wrapper for READ commands
 */
class ReadCmd : public Command
{

  public:

    ReadCmd(Cycle birth_cycle, uint32_t chan, uint32_t rank, uint32_t bank,
            uint32_t row, uint32_t col, uint16_t priority = 0,
            Transaction *tx = nullptr, bool ap = false)
        : Command(birth_cycle)
    {
        type_ = ap ? READ_AP : READ;
        chan_ = chan;
        rank_ = rank;
        bank_ = bank;
        row_ = row;
        col_ = col;
        priority_ = priority;
        parent_tx_ = tx;
    }

};


/*
 * just a wrapper for WRITE commands
 */
class WriteCmd : public Command
{

  public:

    WriteCmd(Cycle birth_cycle, uint32_t chan, uint32_t rank, uint32_t bank,
             uint32_t row, uint32_t col, uint16_t priority = 0,
             Transaction *tx = nullptr, bool ap = false)
        : Command(birth_cycle)
    {
        type_ = ap ? WRITE_AP : WRITE;
        chan_ = chan;
        rank_ = rank;
        bank_ = bank;
        row_ = row;
        col_ = col;
        priority_ = priority;
        parent_tx_ = tx;
    }

};


/*
 * Customized command comparison
 */
class CmdCompare
{

  public:

    bool operator() (const Command *lhs, const Command *rhs) const
    {
        // higher priroity command wins
        if (lhs->priority() > rhs->priority()) return true;
        if (lhs->priority() < rhs->priority()) return false;
        // older command wins
        if (lhs->id() < rhs->id()) return true;
        return false;
    }

};

}

#endif
