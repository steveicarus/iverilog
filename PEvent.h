#ifndef __PEvent_H
#define __PEvent_H
/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: PEvent.h,v 1.1 2000/04/01 19:31:57 steve Exp $"
#endif

# include  "LineInfo.h"
# include  <string>
class ostream;

/*
 * The PEvent class represents event objects. These are things that
 * are declared in Verilog as ``event foo;''
 */
class PEvent : public LineInfo {

    public:
      explicit PEvent(const string&name);
      ~PEvent();

      string name() const;

    private:
      string name_;

    private: // not implemented
      PEvent(const PEvent&);
      PEvent& operator= (const PEvent&);
};

/*
 * $Log: PEvent.h,v $
 * Revision 1.1  2000/04/01 19:31:57  steve
 *  Named events as far as the pform.
 *
 */
#endif
