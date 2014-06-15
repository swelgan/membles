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

#ifndef ADDRESS_MAP_H
#define ADDRESS_MAP_H

#include <vector>

#include "macro.h"
#include "controller_config.h"
#include "device_config.h"

namespace membles
{

class AddressMap
{

  public:
    
    bool init(CtrlCfg *ctrl_cfg, DevCfg *dev_cfg);

    void map(uint64_t addr, uint32_t &chan, uint32_t &rank, uint32_t &bank,
             uint32_t &row, uint32_t &col);

    void info();

  private:

    vector<uint32_t> chan_bits;
    vector<uint32_t> rank_bits;
    vector<uint32_t> bank_bits;
    vector<uint32_t> row_bits;
    vector<uint32_t> col_bits;

    void increment(uint32_t &pos);

    uint32_t extract(uint64_t addr, const vector<uint32_t> &bits);
};

}

#endif
