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

#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>

#include "macro.h"

using namespace std;

namespace membles
{

enum ParamType {
    StringParam,
    IntParam,
    FloatParam,
    BoolParam,
    TimingParam
};

class Parameter
{

  public:

    Parameter();

    bool filled() const { return filled_; }
    void set_filled() { filled_ = true; }

  private:

    bool filled_;

};

class Timing : public Parameter
{

  public:

    Timing(const Timing *ref = NULL);

    Cycle cycle(double tCK) const;
    void set_ns(double ns) { ns_ = ns; set_filled(); }
    void set_cycle(uint32_t cycle) { cycle_ = cycle; set_filled(); }

    friend ostream &operator<<(ostream &os, const Timing &timing);

  private:

    double ns_;
    uint32_t cycle_;

    // reference to another Timing parameter
    // - this is used when a Timing parameter is the alias of another one
    // - e.g. tRFC --> tRFCab
    const Timing *ref_;

};


class CfgItem
{

  public:

    CfgItem(void *ptr, ParamType type);

    bool filled() const { return filled_; }
    bool set(const string &val_str);
    void *get() const { return ptr_; }
    
    friend ostream &operator<<(ostream &os, const CfgItem &cfg_item);

  private:

    void *ptr_;
    ParamType type_;
    bool filled_;

};


class BaseCfg
{

  public:

    BaseCfg();
    virtual ~BaseCfg();

    void create(const string &param_str, void *ptr, ParamType type);
    bool set(string param_str, string val_str);
    bool AllFilled();
    bool ReadFile(const string &filename);
    void print(ofstream &out);

    void set_verbose() { verbose_ = true; }

    virtual void SetDefault() = 0;
    virtual bool check() = 0;

  protected:

    map<string, CfgItem *> conf_map_;
    bool verbose_;

};

}

#endif
