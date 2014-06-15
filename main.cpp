/*
 *  Copyright (c) 2010-2012, Elliott Cooper-Balis
 *                             Paul Rosenfeld
 *                             Bruce Jacob
 *                             University of Maryland
 *                             dramninjas [at] gmail [dot] com
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <getopt.h>

#include "memory_system.h"
#include "transaction.h"

using namespace membles;

void
usage()
{
    cout << "Membles Usage: " << endl;
    cout << "membles -t trace -d spec/device.spec [-s ctrl/system.ctrl] "
         << endl << "        [-o output] [-v] [-h]" << endl;
    cout << "  -t, --trace=FILE                  specify a trace file to run"
         << endl;
    cout << "  -d, --device=FILE1[,FILE2,...]    specify a list of device "
         << "configurations" << endl;
    cout << "  -c, --ctrl=FILE1,[,FILE2,...]     specify a list of controller "
         << "configurations" << endl;
    cout << "  -o, --output=FILE                 specify a file name for all "
         << "the outputs" << endl << "                                      "
         << "e.g. FILE.log, FILE.csv, and FILE.trc" << endl;
    cout << "  -v, --verbose                     enable verbosity" << endl;
    cout << "  -h, --help                        print this message" << endl;
}

bool parse_trace(string line, uint64_t &time, uint64_t &addr, uint32_t &len,
                 bool is_read, uint16_t &priority, void *data)
{
    // skip empty lines
    if (line.empty()) return false;
    // skip comment line
    if (line[0] == '#') return false;
    // get cycle
    size_t space_pos = line.find_first_of(" \t");
    if (space_pos == string::npos) {
        WARN("Insuffient field");
        return false;
    }
    istringstream ss(line.substr(0, space_pos));
    if ((ss >> dec >> time).fail()) {
        WARN("Fail to parse timestamp");
        return false;
    }
    line.erase(0, space_pos + 1);
    // get read/write
    space_pos = line.find_first_of(" \t");
    if (space_pos == string::npos) {
        WARN("Insufficient field");
        return false;
    }
    string rw = to_upper(line.substr(0, space_pos));
    if (rw == "R") {
        is_read = true;
    } else if (rw == "W") {
        is_read = false;
    } else {
        WARN("Fail to parse R/W field " << rw);
        return false;
    }
    line.erase(0, space_pos + 1);
    // get starting address
    space_pos = line.find_first_of(" \t");
    if (space_pos == string::npos) {
        WARN("Insufficient field");
        return false;
    }
    if (line.substr(0, 2) != "0x") {
        WARN("Address should be a hex (starting with 0x)");
        return false;
    }
    ss.clear();
    ss.str(line.substr(2, space_pos - 2));
    if ((ss >> hex >> addr).fail()) {
        WARN("Fail to parse starting address");
        return false;
    }
    line.erase(0, space_pos + 1);
    // get transaction size
    space_pos = line.find_first_of(" \t");
    if (space_pos == string::npos) {
        WARN("Insufficient field");
        return false;
    }
    ss.clear();
    ss.str(line.substr(0, space_pos));
    if ((ss >> dec >> len).fail()) {
        WARN("Fail to parse transaction size");
        return false;
    }
    line.erase(0, space_pos + 1);
    // get priority level
    space_pos = line.find_first_of(" \t");
    ss.clear();
    ss.str(line.substr(0, space_pos));
    if ((ss >> dec >> priority).fail()) {
        WARN("Fail to parse priority level");
        return false;
    }
    line.erase(0, space_pos + 1);


    // TODO: handle data

    // success
    return true;
}

vector<string> parse_dev_filenames(string str)
{
    vector<string> filenames;
    size_t comma_pos = str.find(',');
    while (comma_pos != string::npos) {
        filenames.push_back(str.substr(0, comma_pos));
        str.erase(comma_pos + 1);
        comma_pos = str.find(',');
    }
    filenames.push_back(str);
    return filenames;
}

vector<uint64_t> parse_sizes(string str)
{
    vector<uint64_t> sizes;
    size_t comma_pos = str.find(',');
    while (comma_pos != string::npos) {
        istringstream ss(str.substr(0, comma_pos));
        sizes.push_back(0);
        ss >> sizes.back();
        str.erase(comma_pos + 1);
        comma_pos = str.find(',');
    }
    istringstream ss(str);
    sizes.push_back(0);
    ss >> sizes.back();
    return sizes;
}

int main(int argc, char *argv[])
{
    string trace_filename;
    string ctrl_filename("ctrl/system.ctrl");
    vector<string> dev_filenames;
    vector<uint64_t> mem_sizes;
    string output_prefix;
    bool verbose = false;

    // if user does not specify "-c", then replay the trace to its end
    uint64_t max_cycle = UINT64_MAX;

    // check if the command line carries arguments
    if (argc == 1) {
        usage();
        exit(-1);
    }

    //getopt stuff
    while (1) {
        static struct option long_opts[] = {
            {"trace", required_argument, 0, 't'},
            {"device", required_argument, 0, 'd'},
            {"ctrl", required_argument, 0, 'c'},
            {"output", required_argument, 0, 'o'},
            {"verbose", no_argument, 0, 'v'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };
        int opt_index = 0; //for getopt
        int c = getopt_long(argc, argv, "t:d:c:o:vh", long_opts, &opt_index);
        if (c == -1) break;
        switch (c) {
        case 'h':
            usage();
            exit(0);
            break;
        case 't':
            trace_filename = string(optarg);
            break;
        case 'o':
            output_prefix = string(optarg);
            break;
        case 'c':
            ctrl_filename = string(optarg);
            break;
        case 'd':
            dev_filenames = parse_dev_filenames(optarg);
            break;
        case 'v':
            verbose = true;
            break;
        default:
            usage();
            exit(-1);
        }
    }

    // no default value for the trace input
    if (trace_filename.empty()) {
        ERROR("Please provide a trace input.");
        usage();
        exit(-1);
    }

    // no default value for the device configuration
    if (dev_filenames.size() == 0) {
        ERROR("Please provide at least one device configuration.");
        usage();
        exit(-1);
    }

    // default memory capacity is 1GB
    if (mem_sizes.size() == 0) {
        INFO("Defaulting total memory capacity to 1GB.");
        mem_sizes.push_back(1024);
    }

    // instantiate a Membles
    MemorySystem membles;
    if (verbose) membles.set_verbose();
    if (!membles.init(ctrl_filename, dev_filenames, mem_sizes)) {
        ERROR("Aborted");
        exit(-1);
    }

    Transaction *pending_tx = nullptr;

    // read trace file
    ifstream file;
    string line;
    file.open(trace_filename.c_str());

    if (!file.is_open()) {
        ERROR("Could not open trace file <" << trace_filename << ">.");
        usage();
        exit(-1);
    }

    Cycle next_cycle = 0;

    for (Cycle cycle = 0; cycle < max_cycle; ++cycle) {
        if (!pending_tx && cycle >= next_cycle) {
            if (!file.eof()) {
                getline(file, line);
                uint64_t timestamp = 0;
                uint64_t addr = 0;
                uint32_t len = 0;
                uint16_t priority = 0;
                bool is_read = true;
                bool success = parse_trace(line, timestamp, addr, len, is_read,
                               priority, NULL);
                if (success) {
                    // calculate cycle
                    // timestamp in picosecond, frequency in MHz
                    next_cycle = (Cycle)(timestamp / 1e6 * membles.freq());
                    Transaction *next_tx = new Transaction(addr, len, is_read);
                    if (priority) next_tx->set_priority(priority);
                    if (cycle < next_cycle || !membles.AddTx(next_tx)) {
                        pending_tx = next_tx;
                    }
                }
            }
        } else if (cycle >= next_cycle && membles.AddTx(pending_tx)) {
            pending_tx = nullptr;
        }

        membles.step();

        // quit when trace is fully replayed
        if (file.eof() && !pending_tx && !membles.busy()) break;
    }

    file.close();
    membles.stat();

    // delete remaining pending transactions
    if (pending_tx) delete pending_tx;
    
    // print out completion message
    cout << endl;
    cout << "-------------------------------------------------------" << endl;
    cout << "   Simulation Complete" << endl;
    cout << "   Cycles Elapsed: " << membles.cycle() << endl;
    cout << "-------------------------------------------------------" << endl;
}
