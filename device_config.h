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

#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include "config.h"
#include "controller_config.h"

using namespace std;

namespace membles
{

class DevCfg : public BaseCfg
{

  public:

    DevCfg();
    virtual ~DevCfg() {}

    void SetDefault();
    bool check();

    bool ReadFile(const string &filename);

    bool derive(uint64_t size, const CtrlCfg &ctrl_cfg);

    // function to convert Timing variables into cycle
    Cycle tCCD() const { return max(tCCD_.cycle(tCK), (Cycle)BL / data_rate_); }
    Cycle tRTP() const { return tRTP_.cycle(tCK); }
    Cycle tRCD() const { return tRCD_.cycle(tCK); }
    Cycle tRP() const { return tRPpb_.cycle(tCK); }
    Cycle tRPpb() const { return tRPpb_.cycle(tCK); }
    Cycle tRPab() const { return tRPab_.cycle(tCK); }
    Cycle tRAS() const { return tRAS_.cycle(tCK); }
    Cycle tWR() const { return tWR_.cycle(tCK); }
    Cycle tWTR() const { return tWTR_.cycle(tCK); }
    Cycle tRRD() const { return tRRD_.cycle(tCK); }
    Cycle tFAW() const { return tFAW_.cycle(tCK); }
    Cycle tDQSCK() const { return tDQSCK_.cycle(tCK); }
    Cycle tDQSS() const { return tDQSS_.cycle(tCK); }
    Cycle tRFCab() const { return tRFCab_.cycle(tCK); }
    Cycle tRFCpb() const { return tRFCpb_.cycle(tCK); }
    Cycle tCMD() const { return tCMD_.cycle(tCK); }
    Cycle tRC() const { return tRAS_.cycle(tCK) + tRPab_.cycle(tCK); }
    // aux functions
    Cycle RdToPre() const {
        return AL + BL / data_rate_ + max(tRTP(), tCCD()) - tCCD();
    }
    Cycle WrToPre() const {
        return WL + BL / data_rate_ + tWR() + tDQSS();
    }
    Cycle RdToWr() const {
        // TODO: need to revisit
        return max(RL + BL / data_rate_ + 1 + tDQSCK(), (Cycle)WL) - WL;
    }
    Cycle WrToRd(bool same_rank = true) const {
        if (same_rank) {
            return WL + BL / data_rate_ + tWTR() + tDQSS();
        } else {
            // TODO: need to revisit
            return max(WL + BL / data_rate_ + 1, RL) - RL + tDQSS();
        }
    }

    // public accessible data
    
    // memory type, e.g. DDR3, LPDDR3, ...
    string mem_type;

    // number of banks
    uint32_t num_bank;

    // number of rows
    uint32_t num_row;

    // number of columns
    uint32_t num_col;

    // device width, unit: bit
    uint32_t width;

    // clock period, unit: ns
    double tCK;

    // refresh period, unit: ns
    double tREFI;

    // burst length
    uint32_t BL;

    // data rate, 1=SDR, 2=DDR
    uint32_t data_rate_;

    // read latency
    uint32_t RL;

    // write latency
    uint32_t WL;

    // additional latency used in posted-CAS
    uint32_t AL;

    // other timing parameters
    Timing tCCD_;
    Timing tRTP_;
    Timing tRCD_;
    Timing tRPpb_;
    Timing tRPab_;
    Timing tRAS_;
    Timing tWR_;
    Timing tWTR_;
    Timing tRRD_;
    Timing tFAW_;
    Timing tDQSCK_;
    Timing tDQSS_;
    Timing tRFCab_;
    Timing tRFCpb_;
    Timing tCMD_;

    // supply voltage
    double vdd;
    double vdd_2;

    // idd model file name. If not specified, it's "default", and will use a 
    //  predefined idd file for that specific memory type
    string idd_model;

    // I/O model file name, If not specified, it's "default", and will use a 
    //  predefined I/O file for that specific memory type
    string io_model;

    // idd values
    // Usually there are 1 set of idd for DDRx, and 2 sets for LPDDRx
    double idd0;
    double idd1;
    double idd2p;
    double idd2n;
    double idd3p;
    double idd3n;
    double idd4r;
    double idd4w;
    double idd5;
    double idd6;
    double idd7;
    // only useful when vdd_2 is defined
    double idd0_2;
    double idd1_2;
    double idd2p_2;
    double idd2n_2;
    double idd3p_2;
    double idd3n_2;
    double idd4r_2;
    double idd4w_2;
    double idd5_2;
    double idd6_2;
    double idd7_2;

    // I/O power model-related parameters
    uint32_t dq_per_strobe;
    uint32_t num_cmd_bit;
    uint32_t num_addr_bit;
    // I/O voltage 
    double vdd_io;
    // line capacitance
    double c_line;
    // memory pin capacitance
    double c_mem_dq;
    double c_mem_cmd;
    double c_mem_addr;
    double c_mem_clk;
    // controller pin capacitance
    double c_ctrl_dq;
    double c_ctrl_cmd;
    double c_ctrl_addr;
    double c_ctrl_clk;

    // derived parameters
    
    // number of ranks
    uint32_t num_rank;

    // number of devices per rank
    uint32_t num_device;

    // minimum access length
    uint32_t mal;

};

}

#endif
