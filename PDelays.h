#ifndef __PDelays_H
#define __PDelays_H
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: PDelays.h,v 1.3 2001/01/16 02:44:17 steve Exp $"
#endif

# include  "svector.h"
# include  <string>

#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
class ostream;
#endif

class Design;
class PExpr;

/*
 * Various PForm objects can carry delays. These delays include rise,
 * fall and decay times. This class arranges to carry the triplet.
 */
class PDelays {

    public:
      PDelays();
      ~PDelays();

      void set_delay(PExpr*);
      void set_delays(const svector<PExpr*>*del);

      void eval_delays(Design*des, const string&path,
		       unsigned long&rise_time,
		       unsigned long&fall_time,
		       unsigned long&decay_time) const;

      void dump_delays(ostream&out) const;

    private:
      PExpr* delay_[3];

    private: // not implemented
      PDelays(const PDelays&);
      PDelays& operator= (const PDelays&);
};

ostream& operator << (ostream&o, const PDelays&);

/*
 * $Log: PDelays.h,v $
 * Revision 1.3  2001/01/16 02:44:17  steve
 *  Use the iosfwd header if available.
 *
 * Revision 1.2  2000/02/23 02:56:53  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  1999/09/04 19:11:46  steve
 *  Add support for delayed non-blocking assignments.
 *
 */
#endif
