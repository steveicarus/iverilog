#ifndef __bufif_H
#define __bufif_H
/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: bufif.h,v 1.2 2001/10/31 04:27:46 steve Exp $"
#endif

# include  "functor.h"

class vvp_bufif1_s  : public functor_s {

    public:
      vvp_bufif1_s() : pol(0) {}
      virtual void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
    protected:
      unsigned pol : 1;
};

class vvp_bufif0_s  : public vvp_bufif1_s {

    public:
      vvp_bufif0_s() { pol = 1; }
};

/*
 * $Log: bufif.h,v $
 * Revision 1.2  2001/10/31 04:27:46  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.1  2001/05/31 04:12:43  steve
 *  Make the bufif0 and bufif1 gates strength aware,
 *  and accurately propagate strengths of outputs.
 *
 */
#endif
