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

#include "config.h"

namespace membles
{

/* ctor: Parameter
 * Set filled_ to false
 */
Parameter::Parameter()
    : filled_(false)
{}


/* ctor: Timing
 * Reset both ns_ and cycle_ to zeros
 * If the parameter is jsut an alias, pass the pointer of the actual parameter 
 */
Timing::Timing(const Timing *ref)
    : Parameter(),
      ns_(0.0),
      cycle_(0),
      ref_(ref)
{
    // If the parameter is an alias, it's already set
    if (ref_) set_filled();
}


/*
 * Get the cycle number based on given clock period tCK
 */
Cycle Timing::cycle(double tCK) const
{
    if (ref_) {
        return ref_->cycle(tCK);
    } else {
        return max(cycle_, (uint32_t)ceil(ns_ / tCK));
    }
}

/*
 * Print this timing parameter to an output
 */
ostream &operator<<(ostream &os, const Timing &timing)
{
    if (!timing.filled()) {
        os << "N/A";
    } else {
        double ns = timing.ns_;
        uint32_t cycle = timing.cycle_;
        if (ns != 0.0 && cycle != 0) {
            os << ns << "ns," << cycle;
        } else if (ns != 0.0) {
            os << ns << "ns";
        } else {
            os << cycle;
        }
    }
    return os;
}


/* ctor: CfgItem
 * Set filled to false
 */
CfgItem::CfgItem(void *ptr, ParamType type)
    : ptr_(ptr),
      type_(type),
      filled_(false)
{}


/*
 * Set a value: the data pointer gets allocated first if it is empty
 */
bool CfgItem::set(const string &val_str)
{
    assert(ptr_);
    if (type_ == StringParam) {
        *((string *)ptr_) = val_str;
    } else if (type_ == IntParam) {
        istringstream ss(val_str);
        if ((ss >> dec >> *((uint32_t *)ptr_)).fail()) return false;
    } else if (type_ == FloatParam) {
        istringstream ss(val_str);
        if ((ss >> dec >> *((double *)ptr_)).fail()) return false;
    } else if (type_ == BoolParam) {
        *((bool *)ptr_) = (val_str == "true" || val_str == "1");
    } else if (type_ == TimingParam) {
        // split input string into vectors using ',' delimit
        vector<string> val_strs;
        string temp = val_str;
        size_t pos = temp.find(',');
        while (pos != string::npos) {
            val_strs.push_back(temp.substr(0, pos));
            temp.erase(0, pos + 1);
            pos = temp.find(',');
        };
        val_strs.push_back(temp.substr(0, pos));
        // check if more than 2 fiedls
        if (val_strs.size() > 2) return false;
        // convert to Timing
        for (auto &str : val_strs) {
            istringstream ss(str);
            if (str.find("ns") == string::npos) {
                // this is a cycle value
                uint32_t val = 0;
                if ((ss >> dec >> val).fail()) return false;
                ((Timing *)ptr_)->set_cycle(val);
            } else {
                // this is a ns value
                double val = 0.0;
                if ((ss >> dec >> val).fail()) return false;
                ((Timing *)ptr_)->set_ns(val);
            }
        }
    } else {
        // shouldn't happen
        return false;
    }

    filled_ = true;
    return true;
}


/*
 * Print this configuration item to an output
 */
ostream &operator<<(ostream &os, const CfgItem &cfg_item)
{
    if (!cfg_item.filled()) {
        os << "N/A";
    } else if (cfg_item.type_ == StringParam) {
        os << *((string *)cfg_item.ptr_);
    } else if (cfg_item.type_ == IntParam) {
        os << *((uint32_t *)cfg_item.ptr_);
    } else if (cfg_item.type_ == FloatParam) {
        os << *((double *)cfg_item.ptr_);
    } else if (cfg_item.type_ == BoolParam) {
        if (*((bool *)cfg_item.ptr_)) {
            os << "true";
        } else {
            os << "false";
        }
    } else if (cfg_item.type_ == TimingParam) {
        os << *((Timing *)cfg_item.ptr_);
    }
    return os;
}


/* ctor: BaseCfg
 * Defaulting verbosity to false
 */
BaseCfg::BaseCfg()
    : verbose_(false)
{}


/* dtor: BaseCfg
 * Delete the hashing table
 */
BaseCfg::~BaseCfg()
{
    for (auto iter = conf_map_.begin(); iter != conf_map_.end(); ++iter) {
        delete iter->second;
    }
}


/*
 * Create an entry in the hashing table
 */
void BaseCfg::create(const string &param_str, void *ptr, ParamType type)
{
    conf_map_[to_upper(param_str)] = new CfgItem(ptr, type);
}


/*
 * Assign a value to a parameter
 * Both value and paramter name are strings
 * Return false if the parameter name is not in the map or there is a parsing
 *  error
 */
bool BaseCfg::set(string param_str, string val_str)
{
    // convert parameter name to upper case
    param_str = to_upper(param_str);
    // convert value string to lower case
    val_str = to_lower(val_str);
    // check if in the hashing table
    if (conf_map_.count(param_str) == 0) {
        WARN("Ignore " << param_str << " because it is not a valid parameter "
             "name.");
        return false;
    }
    if (verbose_) {
        INFO("Set " << param_str << " to " << val_str);
    }
    // assign value
    return conf_map_[param_str]->set(val_str);
}


/*
 * Check if all the parameters have been set
 */
bool BaseCfg::AllFilled()
{
    for (auto iter = conf_map_.begin(); iter != conf_map_.end(); ++iter) {
        if (!iter->second->filled()) {
            ERROR(iter->first << " needs a value.");
            return false;
        }
    }
    return true;
}


/*
 * Read an input file
 */
bool BaseCfg::ReadFile(const string &filename)
{
    ifstream file;
    file.open(filename.c_str());
    // check if the file can be opened
    if (!file.is_open()) {
        ERROR("Cannot open file " << filename << ".");
        return false;
    }
    // read file line by line
    size_t line_num = 0;
    while (!file.eof()) {
        line_num++;
        // get next line
        string line;
        getline(file, line);
        // if the filename is a directory, return false
        if (file.bad()) {
            ERROR(filename << " is a directory.");
            return false;
        }
        // remove comment
        size_t comment_pos = line.find('#');
        if (comment_pos != string::npos) {
            line.erase(comment_pos);
        }
        // remove whitespace
        line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
        // skip empty line
        if (line.length() == 0) continue;
        // check if the line has an equal sign
        size_t equal_pos = line.find('=');
        if (equal_pos == string::npos) {
            WARN("Ignore line \'" << line << "\' because \'=\' is missing");
            continue;
        }
        // split line into parameter name and value
        string param_str = line.substr(0, equal_pos);
        string val_str = line.substr(equal_pos + 1);
        // add into hashing table
        bool success = set(param_str, val_str);
        if (!success) {
            WARN("Ignore line \'" << line << "\' because it cannot be parsed");
            continue;
        }
    }
    return true;
}


/*
 * Print this configuration to an output
 */
void BaseCfg::print(ofstream &out)
{
    for (auto iter = conf_map_.begin(); iter != conf_map_.end(); ++iter) {
        out << iter->first << "," << iter->second << endl;
    }
}

}
