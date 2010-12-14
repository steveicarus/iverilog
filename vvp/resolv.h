#ifndef __resolv_H
#define __resolv_H
/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "functor.h"

/*
 * This functor type resolves its inputs using the verilog method of
 * combining signals, and outputs that resolved value. The puller
 * value is also blended with the result. This helps with the
 * implementation of tri0 and tri1, which have pull constants attached.
 */
class resolv_functor_s: public functor_s {

    public:
      explicit resolv_functor_s(unsigned char hiz_value);
      ~resolv_functor_s();

      virtual void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);


    private:
      unsigned char istr[4];
      unsigned char hiz_;
};

#endif
