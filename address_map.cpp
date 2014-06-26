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
#include <cmath>

#include "address_map.h"
#include "controller_config.h"
#include "device_config.h"

namespace membles
{

bool AddressMap::init(CtrlCfg *ctrl_cfg, DevCfg *dev_cfg)
{
    uint32_t log_num_chan = log2(ctrl_cfg->num_chan);
    uint32_t log_num_rank = log2(dev_cfg->num_rank);
    uint32_t log_num_bank = log2(dev_cfg->num_bank);
    uint32_t log_num_row = log2(dev_cfg->num_row);
    uint32_t log_num_col = log2(dev_cfg->num_col);
    if (dev_cfg->width % 8) {
        ERROR("Device width must be a multiple of byte");
        return false;
    }
    uint32_t cur_pos = log2(dev_cfg->width / 8);

    // assign channel bits
    uint32_t chan_bit = ctrl_cfg->chan_itlv_bit;
    for (size_t i = 0; i < log_num_chan; ++i) {
        if (cur_pos == chan_bit) cur_pos++;
        chan_bits.push_back(chan_bit++);
    }

    // assign lower part of the column bit because DRAM is commonly accessed
    //   in burst
    uint32_t log_burst_len = log2(dev_cfg->BL);
    for (size_t i = 0; i < log_burst_len; ++i) {
        col_bits.push_back(cur_pos);
        increment(cur_pos);
    }

    string str(ctrl_cfg->addr_map);
    // parse the address mapping scheme from right to left
    while (!str.empty()) {
        size_t comma_pos = str.find_last_of(",");
        string pattern;
        if (comma_pos == string::npos) {
            // only one pattern left
            pattern = str;
            str.clear();
        } else {
            // extract the last pattern after the found comma
            pattern = str.substr(comma_pos + 1);
            str.erase(comma_pos);
        }
        // parse this pattern
        if (pattern.find("rank") == 0) {
            // parse rank bits
            uint32_t to_fill = log_num_rank - rank_bits.size();
            if (pattern.length() > 4) {
                if ((istringstream(pattern.substr(4)) >> to_fill).fail()) {
                    ERROR("Rank bit pattern \'" << pattern <<
                          "\' is not valid.");
                    return false;
                }
                if (to_fill > log_num_rank - rank_bits.size()) {
                    ERROR("Specified number of rank bits is too large");
                    return false;
                }
            }
            for (size_t i = 0; i < to_fill; ++i) {
                rank_bits.push_back(cur_pos);
                increment(cur_pos);
            }
        } else if (pattern.find("bank") == 0) {
            // parse bank bits
            uint32_t to_fill = log_num_bank - bank_bits.size();
            if (pattern.length() > 4) {
                if ((istringstream(pattern.substr(4)) >> to_fill).fail()) {
                    ERROR("Bank bit pattern \'" << pattern <<
                          "\' is not valid.");
                    return false;
                }
                if (to_fill > log_num_bank - bank_bits.size()) {
                    ERROR("Specified number of bank bits is too large");
                    return false;
                }
            }
            for (size_t i = 0; i < to_fill; ++i) {
                bank_bits.push_back(cur_pos);
                increment(cur_pos);
            }
        } else if (pattern.find("row") == 0) {
            // parse row bits
            uint32_t to_fill = log_num_row - row_bits.size();
            if (pattern.length() > 3) {
                if ((istringstream(pattern.substr(3)) >> to_fill).fail()) {
                    ERROR("Row bit pattern \'" << pattern <<
                          "\' is not valid.");
                    return false;
                }
                if (to_fill > log_num_row - row_bits.size()) {
                    ERROR("Specified number of row bits is too large");
                    return false;
                }
            }
            for (size_t i = 0; i < to_fill; ++i) {
                row_bits.push_back(cur_pos);
                increment(cur_pos);
            }
        } else if (pattern.find("col") == 0) {
            // passe column bits
            uint32_t to_fill = log_num_col - col_bits.size();
            if (pattern.length() > 3) {
                if ((istringstream(pattern.substr(3)) >> to_fill).fail()) {
                    ERROR("Column bit pattern \'" << pattern <<
                          "\' is not valid.");
                    return false;
                }
                if (to_fill > log_num_col - col_bits.size()) {
                    ERROR("Specified number of column bits is too large");
                    return false;
                }
            }
            for (size_t i = 0; i < to_fill; ++i) {
                col_bits.push_back(cur_pos);
                increment(cur_pos);
            }
        }   
    }

    if (chan_bits.size() > 32 || rank_bits.size() > 32 ||
            bank_bits.size() > 32 || row_bits.size() > 32 ||
            col_bits.size() > 32) {
        ERROR("Some part of the address segment (e.g. row, column) is beyond "
              "design limit (i.e. 32-bit).  If this is not caused by an "
              "incorrect setting, please hack the code by yourself");
        return false;
    }

    return true;
}


/*
 * Decode a physical address into memory address segments
 */
void AddressMap::map(uint64_t addr, uint32_t &chan, uint32_t &rank,
                     uint32_t &bank, uint32_t &row, uint32_t &col)
{
    chan = extract(addr, chan_bits);
    rank = extract(addr, rank_bits);
    bank = extract(addr, bank_bits);
    row = extract(addr, row_bits);
    col = extract(addr, col_bits);
}


/*
 * Print out some information about this mapper
 */
void AddressMap::info()
{
    if (chan_bits.size()) {
        cout << "\tChannel bits:\t";
        for (auto &pos : chan_bits) cout << pos << " ";
        cout << endl;
    }
    if (rank_bits.size()) {
        cout << "\tRank bits:\t";
        for (auto &pos : rank_bits) cout << pos << " ";
        cout << endl;
    }
    if (bank_bits.size()) {
        cout << "\tBank bits:\t";
        for (auto &pos : bank_bits) cout << pos << " ";
        cout << endl;
    }
    if (row_bits.size()) {
        cout << "\tRow bits:\t";
        for (auto &pos : row_bits) cout << pos << " ";
        cout << endl;
    }
    if (col_bits.size()) {
        cout << "\tColumn bits:\t";
        for (auto &pos : col_bits) cout << pos << " ";
        cout << endl;
    }
} 


/*
 * Increment the bit pos by 1 and skipping the channel bits
 */
void AddressMap::increment(uint32_t &pos)
{
    do {
        pos++;
    } while (chan_bits.size() && pos >= chan_bits.front() &&
                pos <= chan_bits.back());
}


/*
 * Extract parts of address
 */
uint32_t AddressMap::extract(uint64_t addr, const vector<uint32_t> &bits)
{
    uint32_t ret = 0;
    for (auto iter = bits.rbegin(); iter != bits.rend(); ++iter) {
        ret = (ret << 1) | ((addr >> *iter) & 0x1);
    }
    return ret;
}

}
