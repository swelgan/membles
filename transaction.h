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

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <cstdint>

namespace membles
{

class Transaction
{

  public:

    Transaction(uint64_t addr,
                uint32_t len,
                bool is_read,
                void *data = nullptr);

    uint64_t id() const { return id_; }
    bool is_read() const { return is_read_; }
    uint64_t addr() const { return addr_; }
    uint32_t len() const { return len_; }
    uint16_t priority() const { return priority_; }
    void set_priority(uint16_t priority) { priority_ = priority; }

  protected:

    // transaction ID
    uint64_t id_;
    // transaction starting address
    uint64_t addr_;
    // transaction length
    uint32_t len_;
    // is transaction a read?
    bool is_read_;
    // transaction priority level, 0=lowest
    uint16_t priority_;
    // transaction data, optional
    void *data_;

  private:
  
    static uint64_t count;

};

}

#endif
