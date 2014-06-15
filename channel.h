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

#ifndef CHANNEL_H
#define CHANNEL_H

#include <vector>

#include "base_obj.h"
#include "transaction.h"
#include "address_map.h"
#include "bank.h"
#include "scheduler.h"

namespace membles
{

class Channel : public MemObj
{

  public:

    Channel();
    virtual ~Channel();

    bool init(uint16_t id, CtrlCfg *ctrl_cfg, DevCfg *dev_cfg,
              ofstream *log, ofstream *csv, ofstream *trc);

    void step();

    // accessors
    uint32_t id() const { return id_; }

    bool AddTx(Transaction *tx);

    void stat();

    void process(Command *cmd);

  private:

    // channel id
    uint32_t id_;

    // address mapper
    AddressMap mapper_;
    
    // bank state table
    vector<vector<Bank>> banks_;

    // memory scheduler
    Scheduler sched_;

    // read transaction queue
    vector<Transaction *> rd_queue_;
    // scheduled read tansacitons are moved to read response queue
    vector<Transaction *> rd_resp_queue_;

    // write transaction queue
    vector<Transaction *> wr_queue_;
    // scheduled write tansacitons are moved to write response queue
    vector<Transaction *> wr_resp_queue_;
    
    // indicating whether the channel is in write draining state
    bool wr_draining_;

    // dispatch transaction into scheduler
    bool DispatchTransaction();
    bool DispatchRead();
    bool DispatchWrite();

};

}

#endif
