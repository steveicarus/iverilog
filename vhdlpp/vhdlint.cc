#include "config.h"
#include "vhdlint.h"
#include <cstring>
#include <cstdlib>
#include <sstream>

bool vhdlint::is_negative() const
{
    return value_ < 0L;
}

bool vhdlint::is_positive() const
{
    return value_ > 0L;
}

bool vhdlint::is_zero() const
{
    return value_ == 0L;
}

vhdlint::vhdlint(const char* text)
{
    unsigned text_length = strlen(text);
    if(text_length == 0)
    {
        value_ = 0L;
        return;
    }
    
    char* new_text = new char[text_length + 1];
    
    const char* ptr;
    char* new_ptr;
    for(ptr = text, new_ptr = new_text; *ptr != '\0'; ++ptr)
    {
        if(*ptr == '_')
            continue;
        else
        {
            *new_ptr = *ptr;
            ++new_ptr;
            ++ptr;
        }
    }
    *new_ptr = '\0';
    
    istringstream str(new_text);
    delete[] new_text;
    
    //TODO: check if numbers greater than MAX_INT are handled correctly
    str >> value_;
}

vhdlint::vhdlint(const int64_t& val)
{
    value_ = val;
}

vhdlint::vhdlint(const vhdlint& val)
{
    value_ = val.as_long();
}

int64_t vhdlint::as_long() const
{
    return value_;
}