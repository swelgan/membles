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

#ifndef CONTROLLER_CONFIG_H
#define CONTROLLER_CONFIG_H

#include "config.h"

using namespace std;

namespace membles
{

class CtrlCfg : public BaseCfg
{

  public:

    CtrlCfg();
    virtual ~CtrlCfg() {}

    void SetDefault();
    bool check();

    bool ReadFile(const string &filename);

    // public accessible data
    
    // controller frequency
    uint32_t ctrl_freq;

    // number of channels
    uint32_t num_chan;

    // channel interleave bit (LSB)
    uint32_t chan_itlv_bit;

    // channel width, unit: bit
    uint32_t chan_width;

    // max transaction queue depth
    uint32_t max_rd_queue_depth;
    uint32_t max_wr_queue_depth;

    // max command queue depth
    uint32_t max_cmd_queue_depth;

    // address mapping scheme patterns
    string addr_map;

};

}

#endif
