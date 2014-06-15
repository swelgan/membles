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

#include <sstream>
#include <fstream>
#include <unistd.h>

#include "device_config.h"

#define MAX_PATH 80

namespace membles
{

/* ctor: Device parameter
 * Create a hashing table
 */
DevCfg::DevCfg()
{
    create("MEM_TYPE", &mem_type, StringParam);
    create("NUM_BANK", &num_bank, IntParam);
    create("NUM_ROW", &num_row, IntParam);
    create("NUM_COL", &num_col, IntParam);
    create("DEVICE_WIDTH", &width, IntParam);
    create("tCK", &tCK, FloatParam);
    create("tREFI", &tREFI, FloatParam);
    create("BL", &BL, IntParam);
    create("DATA_RATE", &data_rate_, IntParam);
    create("RL", &RL, IntParam);
    create("WL", &WL, IntParam);
    create("AL", &AL, IntParam);
    create("tCCD", &tCCD_, TimingParam);
    create("tRTP", &tRTP_, TimingParam);
    create("tRCD", &tRCD_, TimingParam);
    create("tRPpb", &tRPpb_, TimingParam);
    create("tRPab", &tRPab_, TimingParam);
    create("tRAS", &tRAS_, TimingParam);
    create("tWR", &tWR_, TimingParam);
    create("tWTR", &tWTR_, TimingParam);
    create("tRRD", &tRRD_, TimingParam);
    create("tFAW", &tFAW_, TimingParam);
    create("tDQSCK", &tDQSCK_, TimingParam);
    create("tDQSS", &tDQSS_, TimingParam);
    create("tRFCab", &tRFCab_, TimingParam);
    create("tRFCpb", &tRFCpb_, TimingParam);
    create("tCMD", &tCMD_, TimingParam);
    create("Vdd", &vdd, FloatParam);
    create("Vdd_2", &vdd_2, FloatParam);
    create("IDD_MODEL", &idd_model, StringParam);
    create("IO_MODEL", &io_model, StringParam);
    create("IDD0", &idd0, FloatParam);
    create("IDD1", &idd1, FloatParam);
    create("IDD2P", &idd2p, FloatParam);
    create("IDD2N", &idd2n, FloatParam);
    create("IDD3P", &idd3p, FloatParam);
    create("IDD3N", &idd3n, FloatParam);
    create("IDD4R", &idd4r, FloatParam);
    create("IDD4W", &idd4w, FloatParam);
    create("IDD5", &idd5, FloatParam);
    create("IDD6", &idd6, FloatParam);
    create("IDD7", &idd7, FloatParam);
    create("IDD0_2", &idd0_2, FloatParam);
    create("IDD1_2", &idd1_2, FloatParam);
    create("IDD2P_2", &idd2p_2, FloatParam);
    create("IDD2N_2", &idd2n_2, FloatParam);
    create("IDD3P_2", &idd3p_2, FloatParam);
    create("IDD3N_2", &idd3n_2, FloatParam);
    create("IDD4R_2", &idd4r_2, FloatParam);
    create("IDD4W_2", &idd4w_2, FloatParam);
    create("IDD5_2", &idd5_2, FloatParam);
    create("IDD6_2", &idd6_2, FloatParam);
    create("IDD7_2", &idd7_2, FloatParam);
    create("DQ_PER_STROBE", &dq_per_strobe, IntParam);
    create("NUM_CMD_BIT", &num_cmd_bit, IntParam);
    create("NUM_ADDR_BIT", &num_addr_bit, IntParam);
    create("Vdd_IO", &vdd_io, FloatParam);
    create("C_LINE", &c_line, FloatParam);
    create("C_MEM_DQ", &c_mem_dq, FloatParam);
    create("C_MEM_CMD", &c_mem_cmd, FloatParam);
    create("C_MEM_ADDR", &c_mem_addr, FloatParam);
    create("C_MEM_CLK", &c_mem_clk, FloatParam);
    create("C_CTRL_DQ", &c_ctrl_dq, FloatParam);
    create("C_CTRL_CMD", &c_ctrl_cmd, FloatParam);
    create("C_CTRL_ADDR", &c_ctrl_addr, FloatParam);
    create("C_CTRL_CLK", &c_ctrl_clk, FloatParam);
    // set default values
    SetDefault();
}


/*
 * Set default values to device-related parameters
 */
void DevCfg::SetDefault()
{
    set("MEM_TYPE",     "DDR3");
    set("DATA_RATE",    "2");
    set("AL",           "0");
    set("tDQSS",        "0");
    set("IDD_MODEL",    "default");
    set("IO_MODEL",     "default");
}


/*
 * Check if device-related parameters are illegal
 */
bool DevCfg::check()
{
    return true;
}


/*
 * Read a device configuration file
 * The file should be a <.spec> file, which specifies all timing parameters
 * The <.spec> file further points to a <.idd> file, which specifies IDD values
 * The <.spec> file further points to a <.io> file, which specifies IO specs
 */
bool DevCfg::ReadFile(const string &filename)
{
    INFO("Read device configuration from <" << filename << "> ...");
    // check if the file extension is .ctrl
    size_t dot_pos = filename.find_last_of(".");
    if (dot_pos == string::npos || filename.substr(dot_pos) != ".spec") {
        ERROR(filename << " is not a <.spec> file.");
        return false;
    }
    // read the file
    bool success = BaseCfg::ReadFile(filename);
    if (!success) return false;
    // read IDD model
    char cwd[MAX_PATH];
    getcwd(cwd, MAX_PATH);
    ostringstream ss;
    if (idd_model == "default") {
        ss << cwd << "/idd/default/" << to_upper(mem_type) << "_default.idd";
    } else {
        ss << cwd << "/idd/" << idd_model;
    }
    string idd_filename = ss.str();
    BaseCfg::ReadFile(idd_filename);
    // read I/O model
    ss.str(string());
    if (io_model == "default") {
        ss << cwd << "/io/default/" << to_upper(mem_type) << "_default.io";
    } else {
        ss << cwd << "/io/" << io_model;
    }
    string io_filename = ss.str();
    BaseCfg::ReadFile(io_filename);
    // successfully read all the inputs
    return true;
}


/*
 * Derive other needed parameters from the given inputs
 */
bool DevCfg::derive(uint64_t size, const CtrlCfg &ctrl_cfg)
{
    // calculate minimum access length (unit: byte)
    mal = ctrl_cfg.chan_width * BL;
    // MAL has to be multiple bytes
    if (mal % 8) {
        ERROR("MAL has to be multiple bytes");
        return false;
    }
    // bit to byte conversion
    mal /= 8;

    uint64_t rank_size = ((uint64_t)num_row * num_col * num_bank *
                         ctrl_cfg.chan_width / 8);
    // Byte to MB conversion
    rank_size >>= 20;

    // calculate the number of ranks
    if (size % rank_size) {
        ERROR("The given channel capacity cannot be partitioned into ranks");
        return false;
    }
    num_rank = size / rank_size;

    // calculate the number of devices per rank
    if (ctrl_cfg.chan_width % width) {
        ERROR("The given channel width cannot be formed using given device");
        return false;
    }
    num_device = ctrl_cfg.chan_width / width;

    return true;
}

}
