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
#if !defined(WINNT)
#ident "$Id: arith.h,v 1.10 2001/10/27 03:22:26 steve Exp $"
#endif

# include  "functor.h"

class vvp_arith_  : public vvp_fobj_s {

    public:
      explicit vvp_arith_(vvp_ipoint_t b, unsigned wid);

    protected:
      vvp_ipoint_t base_;
      unsigned wid_;

    protected:
      void output_x_(bool push);

    private: // not implemented
      vvp_arith_(const vvp_arith_&);
      vvp_arith_& operator= (const vvp_arith_&);
};


/*
 * This class is a mode-42 object for arithmetic operators. Inputs
 * that come in cause the 4-input summation to be calculated, and
 * output functors that are affected cause propagations.
 */
class vvp_arith_div  : public vvp_arith_ {

    public:
      explicit vvp_arith_div(vvp_ipoint_t b, unsigned wid);

      void set(vvp_ipoint_t i, functor_t f, bool push);

    private: // not implemented
      vvp_arith_div(const vvp_arith_div&);
      vvp_arith_div& operator= (const vvp_arith_div&);
};

class vvp_arith_mult  : public vvp_arith_ {

    public:
      explicit vvp_arith_mult(vvp_ipoint_t b, unsigned wid);

      void set(vvp_ipoint_t i, functor_t f, bool push);

    private: // not implemented
      vvp_arith_mult(const vvp_arith_mult&);
      vvp_arith_mult& operator= (const vvp_arith_mult&);
};

class vvp_arith_sum  : public vvp_arith_ {

    public:
      explicit vvp_arith_sum(vvp_ipoint_t b, unsigned wid);
      virtual ~vvp_arith_sum();

      void set(vvp_ipoint_t i, functor_t f, bool push);

    private:
      unsigned long*sum_;
      static const unsigned pagesize = 8*sizeof(unsigned long);

    private: // not implemented
      vvp_arith_sum(const vvp_arith_sum&);
      vvp_arith_sum& operator= (const vvp_arith_sum&);
};

class vvp_arith_sub  : public vvp_arith_ {

    public:
      explicit vvp_arith_sub(vvp_ipoint_t b, unsigned wid);
      virtual ~vvp_arith_sub();

      void set(vvp_ipoint_t i, functor_t f, bool push);

    private:
      unsigned long*sum_;
      static const unsigned pagesize = 8*sizeof(unsigned long);

    private: // not implemented
      vvp_arith_sub(const vvp_arith_sub&);
      vvp_arith_sub& operator= (const vvp_arith_sub&);
};

class vvp_cmp_ge  : public vvp_arith_ {

    public:
      explicit vvp_cmp_ge(vvp_ipoint_t b, unsigned wid);

      void set(vvp_ipoint_t i, functor_t f, bool push);

    private: // not implemented
      vvp_cmp_ge(const vvp_cmp_ge&);
      vvp_cmp_ge& operator= (const vvp_cmp_ge&);
};

class vvp_cmp_gt  : public vvp_arith_ {

    public:
      explicit vvp_cmp_gt(vvp_ipoint_t b, unsigned wid);

      void set(vvp_ipoint_t i, functor_t f, bool push);

    private: // not implemented
      vvp_cmp_gt(const vvp_cmp_gt&);
      vvp_cmp_gt& operator= (const vvp_cmp_gt&);
};

class vvp_shiftl  : public vvp_arith_ {

    public:
      explicit vvp_shiftl(vvp_ipoint_t b, unsigned wid);

      void set(vvp_ipoint_t i, functor_t f, bool push);

    private:
      unsigned amount_;

    private: // not implemented
      vvp_shiftl(const vvp_shiftl&);
      vvp_shiftl& operator= (const vvp_shiftl&);
};

class vvp_shiftr  : public vvp_arith_ {

    public:
      explicit vvp_shiftr(vvp_ipoint_t b, unsigned wid);

      void set(vvp_ipoint_t i, functor_t f, bool push);

    private:
      unsigned amount_;

    private: // not implemented
      vvp_shiftr(const vvp_shiftr&);
      vvp_shiftr& operator= (const vvp_shiftr&);
};

/*
 * $Log: arith.h,v $
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
