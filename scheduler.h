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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <set>

#include "base_obj.h"
#include "address_map.h"
#include "command.h"
#include "bank.h"

namespace membles
{

class Channel;

class Scheduler : public MemObj
{

  public:
   
    Scheduler(Channel *parent, AddressMap &mapper, vector<vector<Bank>> &bank);

    bool init(CtrlCfg *ctrl_cfg, DevCfg *dev_cfg,
              ofstream *log, ofstream *csv, ofstream *trc);

    void step();

    bool AddTx(Transaction *tx, bool need_act = false, bool need_pre = false);

    Command *schedule();

    void execute(Command *cmd);

    void SetCmdQueueDepth(uint32_t max_cmd_queue_depth);

  private:

    // pointer to its parent channel
    Channel *parent_;

    // command queue depth
    uint32_t max_cmd_queue_depth_;

    // command queue
    // using STL set instead of STL vector because we want to maintain the 
    //   command order at any time
    set<Command *, CmdCompare> cmd_queue_;

    // address mapper reference
    AddressMap &mapper_;
    
    // bank state table reference
    vector<vector<Bank>> &banks_;

};

}

#endif
