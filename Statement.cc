/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: Statement.cc,v 1.1 1998/11/03 23:28:55 steve Exp $"
#endif

# include  "Statement.h"
# include  "PExpr.h"

Statement::~Statement()
{
}

PBlock::PBlock(BL_TYPE t, const list<Statement*>&st)
: bl_type_(t)
{
      nlist_ = st.size();
      list_ = new Statement*[nlist_];

      list<Statement*>::const_iterator s = st.begin();
      for (unsigned idx = 0 ; s != st.end() ;  s ++, idx += 1 ) {
	    list_[idx] = *s;
      }
}

PBlock::~PBlock()
{
      for (unsigned idx = 0 ;  idx < nlist_ ;  idx += 1)
	    delete list_[idx];

      delete[]list_;
}

PCallTask::PCallTask(const string&n, const list<PExpr*>&p)
: name_(n), nparms_(p.size()), parms_(nparms_?new PExpr*[nparms_]:0)
{
      list<PExpr*>::const_iterator s = p.begin();
      for (unsigned idx = 0 ;  s != p.end() ;  s++, idx += 1) {
	    parms_[idx] = *s;
      }

}


/*
 * $Log: Statement.cc,v $
 * Revision 1.1  1998/11/03 23:28:55  steve
 *  Introduce verilog to CVS.
 *
 */

