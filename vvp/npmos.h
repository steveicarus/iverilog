#ifndef __npmos_H
#define __npmos_H
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
#ident "$Id: npmos.h,v 1.2 2001/10/18 17:30:26 steve Exp $"
#endif

# include  "functor.h"

class vvp_pmos_s  : public vvp_fobj_s {

    public:
      virtual void set(vvp_ipoint_t i, functor_t f, bool push);

    private: // not implemented
};

class vvp_nmos_s  : public vvp_fobj_s {

    public:
      virtual void set(vvp_ipoint_t i, functor_t f, bool push);

    private: // not implemented
};

class vvp_rpmos_s  : public vvp_fobj_s {

    public:
      virtual void set(vvp_ipoint_t i, functor_t f, bool push);

    private: // not implemented
};

class vvp_rnmos_s  : public vvp_fobj_s {

    public:
      virtual void set(vvp_ipoint_t i, functor_t f, bool push);

    private: // not implemented
};

/*
 * $Log: npmos.h,v $
 * Revision 1.2  2001/10/18 17:30:26  steve
 *  Support rnpmos devices. (Philip Blundell)
 * Revision 1.1 2001/10/09 02:28:17 steve Add the
 * PMOS and NMOS functor types.
 *
 * Revision 1.1  2001/05/31 04:12:43  steve
 *  Make the bufif0 and bufif1 gates strength aware,
 *  and accurately propagate strengths of outputs.
 *
 */
#endif
