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
#ident "$Id: PTask.cc,v 1.7 2002/08/12 01:34:58 steve Exp $"
#endif

# include "config.h"

# include  "PTask.h"

PTask::PTask()
: ports_(0), statement_(0)
{
}

PTask::~PTask()
{
}

void PTask::set_ports(svector<PWire*>*p)
{
      assert(ports_ == 0);
      ports_ = p;
}

void PTask::set_statement(Statement*s)
{
      assert(statement_ == 0);
      statement_ = s;
}


/*
 * $Log: PTask.cc,v $
 * Revision 1.7  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.6  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.5  2001/04/19 03:04:47  steve
 *  Spurious assert of empty statemnt.
 *
 * Revision 1.4  2001/01/13 22:20:08  steve
 *  Parse parameters within nested scopes.
 *
 * Revision 1.3  2000/02/23 02:56:53  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.2  1999/07/24 02:11:19  steve
 *  Elaborate task input ports.
 *
 * Revision 1.1  1999/07/03 02:12:51  steve
 *  Elaborate user defined tasks.
 *
 */

