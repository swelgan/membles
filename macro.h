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

#ifndef MACRO_H
#define MACRO_H

#include <cassert>
#include <iostream>
#include <algorithm>
#include <string>
#include <cmath>

using namespace std;

#define ERROR(str) { \
    cerr << "[ERROR] " << str << endl; \
}

#define WARN(str) { \
    cerr << "[WARN] " << str << endl; \
}

#define INFO(str) { \
    cout << "[INFO] " << str << endl; \
}

namespace membles
{

typedef uint64_t Cycle;

#define MAX_CYCLE UINT64_MAX

typedef uint64_t Frequency;


/*
 * upper case conversion
 */
inline string to_upper(string str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}


/*
 * lower case conversion
 */
inline string to_lower(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}


/*
 * address alignment
 */
inline void align(uint64_t &addr, uint32_t &len, uint32_t gran) {
    uint32_t log_gran = log2(gran);
    uint64_t start_addr = (addr >> log_gran) << log_gran;
    uint64_t end_addr = (((addr + len - 1) >> log_gran) << log_gran) + gran;
    len = end_addr - start_addr;
    addr = start_addr;
}
}

#endif
