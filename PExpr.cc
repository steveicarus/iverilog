/*
 * Copyright (c) 1998 Stephen Williams <steve@icarus.com>
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
#ident "$Id: PExpr.cc,v 1.2 1998/11/11 00:01:51 steve Exp $"
#endif

# include  "PExpr.h"
# include  <typeinfo>

PExpr::~PExpr()
{
}

bool PExpr::is_the_same(const PExpr*that) const
{
      return typeid(this) == typeid(that);
}

bool PENumber::is_the_same(const PExpr*that) const
{
      const PENumber*obj = dynamic_cast<const PENumber*>(that);
      if (obj == 0)
	    return false;

      return *value_ == *obj->value_;
}

/*
 * $Log: PExpr.cc,v $
 * Revision 1.2  1998/11/11 00:01:51  steve
 *  Check net ranges in declarations.
 *
 * Revision 1.1  1998/11/03 23:28:53  steve
 *  Introduce verilog to CVS.
 *
 */

