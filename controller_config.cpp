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

#include <fstream>
#include <sstream>

#include "controller_config.h"

namespace membles
{

/* ctor: Controller parameter
 * Create a hashing map
 */
CtrlCfg::CtrlCfg()
{
    create("CTRL_FREQ", &ctrl_freq, IntParam);
    create("NUM_CHAN", &num_chan, IntParam);
    create("CHAN_INTERLEAVE_BIT", &chan_itlv_bit, IntParam);
    create("DATA_BUS_BIT", &chan_width, IntParam);
    create("READ_TRANS_QUEUE", &max_rd_queue_depth, IntParam);
    create("WRITE_TRANS_QUEUE", &max_wr_queue_depth, IntParam);
    create("CMD_QUEUE", &max_cmd_queue_depth, IntParam);
    create("ADDR_MAP", &addr_map, StringParam);

    SetDefault();
}


/*
 * Set default values to controller-related parameters
 */
void CtrlCfg::SetDefault()
{
    set("CTRL_FREQ",            "800"   );
    set("NUM_CHAN",             "1"     );
    set("CHAN_INTERLEAVE_BIT",  "10"    );
    set("READ_TRANS_QUEUE",     "8"     );
    set("WRITE_TRANS_QUEUE",    "8"     );
    set("CMD_QUEUE",            "16"    );
}


/*
 * Check if controller-related parameters are illegal
 */
bool CtrlCfg::check()
{
    return true;
}


/*
 * Read a controller configuration file
 * The file should be a <.ctrl> file
 */
bool CtrlCfg::ReadFile(const string &filename)
{
    INFO("Read controller configuration from <" << filename << "> ...");
    // check if the file extension is .ctrl
    size_t dot_pos = filename.find_last_of(".");
    if (dot_pos == string::npos || filename.substr(dot_pos) != ".ctrl") {
        ERROR(filename << " is not a <.ctrl> file.");
        return false;
    }
    // read the file
    return BaseCfg::ReadFile(filename);
}

}
