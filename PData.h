#ifndef __PData_H
#define __PData_H
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
#ident "$Id: PData.h,v 1.1 2003/01/26 21:15:58 steve Exp $"
#endif

# include  "HName.h"
# include  "netlist.h"
# include  "LineInfo.h"

/*
 * The PData object represents declaration of atomic datum such as
 * real and realtime variables. These are variables that cannot be bit
 * or part selected, but can be used in expressions.
 */

class PData : public LineInfo {

    public:
      PData(const hname_t&hname);
      ~PData();

	// Return a hierarchical name.
      const hname_t&name() const;

      void elaborate_scope(Design*des, NetScope*scope) const;

      map<string,PExpr*> attributes;

    private:
      hname_t hname_;

    private:
      PData(const PData&);
      PData& operator= (const PData&);
};

/*
 * $Log: PData.h,v $
 * Revision 1.1  2003/01/26 21:15:58  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 */
#endif
