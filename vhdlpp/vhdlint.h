#ifndef __vhdlint_H
#define __vhdlint_H

#include "config.h"
#include <stdint.h>

using namespace std;

class vhdlint
{
    public:
        explicit vhdlint(const char* text);
        explicit vhdlint(const int64_t& val);
        explicit vhdlint(const vhdlint& val);
        
        bool is_negative() const;
        bool is_positive() const;
        bool is_zero() const;
        
        int64_t as_long() const;
        //vhdlv get(const unsigned index) const;
        //void set(const unsigned index, const unsigned val);
       // unsigned short operator[](const unsigned index);
    private:
        int64_t value_;
};

#endif
