#ifndef __PSpec_H
#define __PSpec_H
/*
 * Copyright (c) 2006 Stephen Williams <steve@icarus.com>
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
#ident "$Id: PSpec.h,v 1.1 2006/09/23 04:57:19 steve Exp $"
#endif

# include  "LineInfo.h"
# include  "StringHeap.h"
# include  <vector>

class PSpecPath  : public LineInfo {

    public:
      PSpecPath(unsigned src_cnt, unsigned dst_cnt);
      ~PSpecPath();

      void elaborate(class Design*des, class NetScope*scope) const;

      void dump(std::ostream&out, unsigned ind) const;

    public:
	// Ordered set of source nodes of a path
      std::vector<perm_string> src;
	// Ordered set of destination nodes of a path
      std::vector<perm_string> dst;

      std::vector<class PExpr*>delays;
};

#endif
