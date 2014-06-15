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

#ifndef BASE_OBJ_H
#define BASE_OBJ_H

#include <fstream>

#include "controller_config.h"
#include "device_config.h"

namespace membles
{

class BaseObj
{

  public:

    BaseObj()
        : log_(NULL),
          csv_(NULL),
          trc_(NULL),
          cycle_(0),
          busy_(false),
          verbose_(false)
    {}

    virtual void step() = 0;

    Cycle cycle() const { return cycle_; }
    bool busy() const { return busy_; }
    void set_verbose() { verbose_ = true; }

  protected:

    // log output
    ofstream *log_;
    // csv output
    ofstream *csv_;
    // trace output
    ofstream *trc_;
    // current simulated cycle
    Cycle cycle_;
    // busy doing something
    bool busy_;
    // verbosity
    bool verbose_;

};


class MemObj : public BaseObj
{

  public:

    MemObj()
        : BaseObj(),
          ctrl_cfg_(NULL),
          dev_cfg_(NULL)
    {}

    bool init(CtrlCfg *ctrl_cfg, DevCfg *dev_cfg,
              ofstream *log, ofstream *csv, ofstream *trc);

  protected:

    // controller configuration (global)
    CtrlCfg *ctrl_cfg_;
    // device configuration (per-channel)
    DevCfg *dev_cfg_;

};

}

#endif
