/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#if !defined(WINNT)
#ident "$Id: PFunction.cc,v 1.2 1999/08/25 22:22:41 steve Exp $"
#endif

#include "PTask.h"

PFunction::PFunction(svector<PWire*>*p, Statement*s)
: out_(0), ports_(p), statement_(s)
{
}

PFunction::~PFunction()
{
}

void PFunction::set_output(PWire*o)
{
      assert(out_ == 0);
      out_ = o;
}

/*
 * $Log: PFunction.cc,v $
 * Revision 1.2  1999/08/25 22:22:41  steve
 *  elaborate some aspects of functions.
 *
 */
