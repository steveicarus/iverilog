/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: PData.cc,v 1.2 2003/11/10 20:11:01 steve Exp $"
#endif

# include  "config.h"
# include  "PData.h"

PData::PData(const hname_t&h)
: hname_(h)
{
}

PData::~PData()
{
}

const hname_t&PData::name() const
{
      return hname_;
}

/*
 * $Log: PData.cc,v $
 * Revision 1.2  2003/11/10 20:11:01  steve
 *  missing include of config.h
 *
 * Revision 1.1  2003/01/26 21:15:58  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 */

