#ifndef __vhdlreal_h
#define __vhdlreal_h

#include "config.h"
#include <iostream>
#include <cmath>

using namespace std;
/*
* This class holds a floating point decimal number. The number is
* stored as double. All based numbers are converted by an external
* function to a double and then stored as class instance.
*/
class vhdlreal
{
public: 
    friend ostream& operator<< (ostream&, const vhdlreal&);
    friend vhdlreal operator+ (const vhdlreal&, const vhdlreal&);
    friend vhdlreal operator- (const vhdlreal&, const vhdlreal&);
    friend vhdlreal operator* (const vhdlreal&, const vhdlreal&);
    friend vhdlreal operator/ (const vhdlreal&, const vhdlreal&);
    friend vhdlreal operator% (const vhdlreal&, const vhdlreal&);
    friend vhdlreal pow(const vhdlreal&, const vhdlreal&);
    // Unary minus.
    friend vhdlreal operator- (const vhdlreal&);

    explicit vhdlreal();
    explicit vhdlreal(const char*text);
    explicit vhdlreal(const double& val);
    vhdlreal(const vhdlreal& val);
    virtual ~vhdlreal() {};
    
    double as_double() const
    {
        return value_;
    }
protected:
    double value_;
};
#endif