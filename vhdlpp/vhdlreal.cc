#include "config.h"
#include "compiler.h"
#include "vhdlreal.h"
#include <assert.h>
#include <cstring>
#include <cstdlib>

vhdlreal::vhdlreal() {
    value_ = 0.0;
}

vhdlreal::vhdlreal(const double& r) {
    value_ = r;
}

vhdlreal::vhdlreal(const vhdlreal& val) {
    value_ = val.as_double();
}

vhdlreal::vhdlreal(const char* text) {
    assert(strlen(text) != 0);
    char* buffer = new char[strlen(text)+1];
    
    char* buf_ptr;
    for(buf_ptr = buffer; *text != 0; ++buf_ptr, ++text) {
        if(*text == '_')
            continue;
        *buf_ptr = *text;
    }
    *buf_ptr = '\0';
    
    value_ = strtod(buffer, NULL);
    delete[] buffer;
}

ostream& operator<< (ostream& str, const vhdlreal& r) {
    return (str << r.as_double());
}
vhdlreal operator+ (const vhdlreal& r1, const vhdlreal& r2) {
    return vhdlreal(r1.as_double() + r2.as_double());
}
vhdlreal operator- (const vhdlreal& r1, const vhdlreal& r2) {
    return vhdlreal(r1.as_double() - r2.as_double());
}
vhdlreal operator* (const vhdlreal& r1, const vhdlreal& r2) {
    return vhdlreal(r1.as_double() * r2.as_double());
}
vhdlreal operator/ (const vhdlreal& r1, const vhdlreal& r2) {
    return vhdlreal(r1.as_double() / r2.as_double());
}
vhdlreal operator% (const vhdlreal& r1, const vhdlreal& r2) {
    return vhdlreal(fmod(r1.as_double(), r2.as_double()));
}
vhdlreal pow(const vhdlreal& r1, const vhdlreal& r2) {
    return vhdlreal(pow(r1.as_double(), r2.as_double()));
}
vhdlreal operator- (const vhdlreal& r) {
    return vhdlreal(-r.as_double());
}