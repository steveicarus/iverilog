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
#ifdef HAVE_CVS_IDENT
#ident "$Id: PFunction.cc,v 1.7 2004/05/31 23:34:36 steve Exp $"
#endif

# include "config.h"

#include "PTask.h"

PFunction::PFunction(perm_string name)
: name_(name), ports_(0), statement_(0)
{
      return_type_.type = PTF_NONE;
}

PFunction::~PFunction()
{
}

void PFunction::set_ports(svector<PWire *>*p)
{
      assert(ports_ == 0);
      ports_ = p;
}

void PFunction::set_statement(Statement*s)
{
      assert(s != 0);
      assert(statement_ == 0);
      statement_ = s;
}

void PFunction::set_return(PTaskFuncArg t)
{
      return_type_ = t;
}

/*
 * $Log: PFunction.cc,v $
 * Revision 1.7  2004/05/31 23:34:36  steve
 *  Rewire/generalize parsing an elaboration of
 *  function return values to allow for better
 *  speed and more type support.
 *
 * Revision 1.6  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.4  2001/01/13 22:20:08  steve
 *  Parse parameters within nested scopes.
 *
 * Revision 1.3  2000/02/23 02:56:53  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.2  1999/08/25 22:22:41  steve
 *  elaborate some aspects of functions.
 *
 */
