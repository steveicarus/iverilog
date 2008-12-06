#ifndef __PDelays_H
#define __PDelays_H
/*
 * Copyright (c) 1999-2002 Stephen Williams (steve@icarus.com)
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

# include  "svector.h"
# include  <string>
# include  <iostream>

#ifdef __GNUC__
#if __GNUC__ > 2
using namespace std;
#endif
#endif

class Design;
class NetScope;
class NetExpr;
class PExpr;

/*
 * Various PForm objects can carry delays. These delays include rise,
 * fall and decay times. This class arranges to carry the triplet.
 */
class PDelays {

    public:
      PDelays();
      ~PDelays();

	/* Set the delay expressions. If the delete_flag is true, then
	   this object takes ownership of the expressions, and will
	   delete it in the destructor. */
      void set_delay(PExpr*);
      void set_delays(const svector<PExpr*>*del, bool delete_flag=true);

      void eval_delays(Design*des, NetScope*scope,
		       NetExpr*&rise_time,
		       NetExpr*&fall_time,
		       NetExpr*&decay_time,
		       bool as_nets_flag =false) const;

      void dump_delays(ostream&out) const;

    private:
      PExpr* delay_[3];
      bool delete_flag_;

    private: // not implemented
      PDelays(const PDelays&);
      PDelays& operator= (const PDelays&);
};

ostream& operator << (ostream&o, const PDelays&);

#endif
