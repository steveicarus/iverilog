/*
 * Copyright (c) 2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: PEvent.cc,v 1.5 2004/02/19 06:57:10 steve Exp $"
#endif

# include "config.h"

# include  "PEvent.h"

PEvent::PEvent(perm_string n)
: name_(n)
{
}

PEvent::~PEvent()
{
}

perm_string  PEvent::name() const
{
      return name_;
}

/*
 * $Log: PEvent.cc,v $
 * Revision 1.5  2004/02/19 06:57:10  steve
 *  Memory and Event names use perm_string.
 *
 * Revision 1.4  2003/03/01 06:25:30  steve
 *  Add the lex_strings string handler, and put
 *  scope names and system task/function names
 *  into this table. Also, permallocate event
 *  names from the beginning.
 *
 * Revision 1.3  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.1  2000/04/01 19:31:57  steve
 *  Named events as far as the pform.
 *
 */

