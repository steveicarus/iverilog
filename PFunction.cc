#include "PTask.h"

PFunction::PFunction(svector<PWire*>*p, Statement*s)
: ports_(p), statement_(s)
{
}

PFunction::~PFunction()
{
}
