#ifndef __force_H
#define __force_H
/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
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

#include "functor.h"

/*
 *   Force functors
 */

class force_functor_s : public functor_s
{
public:
      force_functor_s() : active(0) {}
      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
      unsigned active : 1;
};


/*
 *   Variable functors
 */

struct var_functor_s: public functor_s {
      virtual void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

      bool assigned() { return !!port[3]; };
      bool deassign(vvp_ipoint_t itgt);
};

#endif
