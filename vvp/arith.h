#ifndef __arith_H
#define __arith_H
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: arith.h,v 1.15 2003/04/11 05:15:38 steve Exp $"
#endif

# include  "functor.h"

// base class for arithmetic functors

class vvp_arith_  : public functor_s {

    public:
      explicit vvp_arith_(unsigned wid) : wid_(wid) {};

    protected:
      unsigned wid_;

    protected:
      void output_x_(vvp_ipoint_t base, bool push, unsigned val = 2);
      void output_val_(vvp_ipoint_t base, bool push, unsigned long sum);
};

// base class for wide arithmetic functors

class vvp_wide_arith_  : public vvp_arith_ {
    public:
      explicit vvp_wide_arith_(unsigned wid); 

    protected:
      static const unsigned pagesize = 8*sizeof(unsigned long);
      unsigned pagecount_;
      unsigned long *sum_;

      void output_val_(vvp_ipoint_t base, bool push);
};

/*
 * This class is functor for arithmetic sum. Inputs that come
 * in cause the 4-input summation to be calculated, and output
 * functors that are affected cause propagations.
 */
class vvp_arith_mult  : public vvp_arith_ {

    public:
      explicit vvp_arith_mult(unsigned wid) : vvp_arith_(wid) {}

      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
      void wide(vvp_ipoint_t base, bool push);
};

class vvp_arith_div : public vvp_arith_ {

    public:
      explicit vvp_arith_div(unsigned wid) : vvp_arith_(wid) {}

      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
      void wide(vvp_ipoint_t base, bool push);
};

class vvp_arith_mod : public vvp_arith_ {

    public:
      explicit vvp_arith_mod(unsigned wid) : vvp_arith_(wid) {}

      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
      void wide(vvp_ipoint_t base, bool push);
};

class vvp_arith_sum  : public vvp_wide_arith_ {

    public:
      explicit vvp_arith_sum(unsigned wid) : vvp_wide_arith_(wid) {}

      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
};

class vvp_arith_sub  : public vvp_wide_arith_ {

    public:
      explicit vvp_arith_sub(unsigned wid) : vvp_wide_arith_(wid) {}

      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
};

class vvp_cmp_ge  : public vvp_arith_ {

    public:
      explicit vvp_cmp_ge(unsigned wid, bool signed_flag);
      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

    private:
      bool signed_flag_;
};

class vvp_cmp_gt  : public vvp_arith_ {

    public:
      explicit vvp_cmp_gt(unsigned wid, bool signed_flag);
      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

    private:
      bool signed_flag_;
};

class vvp_shiftl  : public vvp_arith_ {

    public:
      explicit vvp_shiftl(unsigned wid) : vvp_arith_(wid) {}

      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
};

class vvp_shiftr  : public vvp_arith_ {

    public:
      explicit vvp_shiftr(unsigned wid) : vvp_arith_(wid) {}

      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
};

/*
 * $Log: arith.h,v $
 * Revision 1.15  2003/04/11 05:15:38  steve
 *  Add signed versions of .cmp/gt/ge
 *
 * Revision 1.14  2002/08/12 01:35:07  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.13  2002/05/07 04:15:43  steve
 *  Fix uninitialized memory accesses.
 *
 * Revision 1.12  2002/01/03 04:19:02  steve
 *  Add structural modulus support down to vvp.
 *
 * Revision 1.11  2001/10/31 04:27:46  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.10  2001/10/27 03:22:26  steve
 *  Minor rework of summation carry propagation (Stephan Boettcher)
 *
 * Revision 1.9  2001/10/16 02:47:37  steve
 *  Add arith/div object.
 *
 * Revision 1.8  2001/07/13 00:38:57  steve
 *  Remove width restriction on subtraction.
 *
 * Revision 1.7  2001/07/07 02:57:33  steve
 *  Add the .shift/r functor.
 *
 * Revision 1.6  2001/07/06 04:46:44  steve
 *  Add structural left shift (.shift/l)
 *
 * Revision 1.5  2001/06/29 01:20:20  steve
 *  Relax limit on width of structural sum.
 *
 * Revision 1.4  2001/06/16 23:45:05  steve
 *  Add support for structural multiply in t-dll.
 *  Add code generators and vvp support for both
 *  structural and behavioral multiply.
 *
 * Revision 1.3  2001/06/15 04:07:58  steve
 *  Add .cmp statements for structural comparison.
 *
 * Revision 1.2  2001/06/07 03:09:03  steve
 *  Implement .arith/sub subtraction.
 *
 * Revision 1.1  2001/06/05 03:05:41  steve
 *  Add structural addition.
 *
 */
#endif
