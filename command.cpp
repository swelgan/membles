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

#include "command.h"

namespace membles
{

uint64_t Command::count = 0;

/* ctor: Command
 * Set the birth cycle and ID
 * Initialze everything else to zero
 */
Command::Command(Cycle birth_cycle)
    : id_(count++),
      birth_cycle_(birth_cycle),
      chan_(0),
      rank_(0),
      bank_(0),
      row_(0),
      col_(0),
      priority_(0),
      parent_tx_(nullptr)
{}


/*
 * Print this command to an output
 */
ostream &operator<<(ostream &os, const Command &cmd)
{
    if (cmd.type() == ACTIVATE) {
        os << "[ACT] CH" << cmd.chan() << " R" << cmd.rank() << " B"
            << cmd.bank() << " r" << cmd.row();
    } else if (cmd.type() == PRECHARGE) {
        os << "[PRE] CH" << cmd.chan() << " R" << cmd.rank() << " B"
            << cmd.bank();
    } else if (cmd.type() == READ) {
        os << "[READ] CH" << cmd.chan() << " R" << cmd.rank() << " B"
            << cmd.bank() << " r" << cmd.row() << " c" << cmd.col();
    } else if (cmd.type() == WRITE) {
        os << "[WRITE] CH" << cmd.chan() << " R" << cmd.rank() << " B"
            << cmd.bank() << " r" << cmd.row() << " c" << cmd.col();
    } else if (cmd.type() == REFRESH) {
        os << "[REFRESH] CH" << cmd.chan() << " R" << cmd.rank();
    } else {
        os << "[UNKNOWN]";
    }
    return os;
}

}
