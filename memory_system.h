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

#ifndef MEMORY_SYSTEM_H
#define MEMORY_SYSTEM_H

#include <string>
#include <vector>
#include <fstream>

#include "macro.h"
#include "base_obj.h"
#include "channel.h"
#include "transaction.h"

using namespace std;

namespace membles
{

class MemorySystem : public BaseObj
{

  public:

    MemorySystem();
    virtual ~MemorySystem();

    bool init(const string &ctrl_filename,
              const vector<string> &dev_filenames,
              vector<uint64_t> sizes);

    void step();

    uint32_t FindChanId(Transaction *tx);

    bool AddTx(Transaction *tx);

    void stat();

    Frequency freq() const { return freq_; }
    void set_verbose();

  private:

    // memory controller frequency, unit: MHz
    Frequency freq_;

    uint32_t num_chan_;
    vector<uint64_t> sizes_;

    // channel interleave bit (LSB), default: bit-10 --> 2KB interleaving
    uint32_t chan_itlv_bit_;

    CtrlCfg ctrl_cfg_;
    vector<DevCfg> dev_cfgs_;

    // components
    vector<Channel> channels_;
};

}

#endif
