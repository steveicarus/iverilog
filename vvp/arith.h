#ifndef __arith_H
#define __arith_H
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
      explicit vvp_arith_div(unsigned wid, bool signed_flag)
      : vvp_arith_(wid), signed_flag_(signed_flag) {}

      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
      void wide(vvp_ipoint_t base, bool push);

    private:
      bool signed_flag_;
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

class vvp_cmp_eq  : public vvp_arith_ {

    public:
      explicit vvp_cmp_eq(unsigned wid);
      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

};

class vvp_cmp_ne  : public vvp_arith_ {

    public:
      explicit vvp_cmp_ne(unsigned wid);
      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

};

class vvp_cmp_gtge_base_ : public vvp_arith_ {

    public:
      explicit vvp_cmp_gtge_base_(unsigned wid, bool signed_flag);

    protected:
      void set_base(vvp_ipoint_t i, bool push, unsigned val, unsigned str,
		    unsigned out_if_equal);
    private:
      bool signed_flag_;
};

class vvp_cmp_ge  : public vvp_cmp_gtge_base_ {

    public:
      explicit vvp_cmp_ge(unsigned wid, bool signed_flag);
      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

    private:
};

class vvp_cmp_gt  : public vvp_cmp_gtge_base_ {

    public:
      explicit vvp_cmp_gt(unsigned wid, bool signed_flag);
      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

    private:
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

#endif
