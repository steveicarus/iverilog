#ifndef __sref_H
#define __sref_H
/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version. In order to redistribute the software in
 *    binary form, you will need a Picture Elements Binary Software
 *    License.
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
#ident "$Id: sref.h,v 1.2 1999/07/18 05:52:47 steve Exp $"
#endif

# include  <assert.h>
# include  "svector.h"

/*
 * The sref class is a reference with automatic reference counting. It
 * implementes a many-to-one linkage where T1 is the many type and T2
 * is the one type.
 */

template <class T1, class T2> class sref;
template <class T1, class T2> class sref_back;

template <class T1, class T2> class sref_back {

      friend class sref<T1,T2>;

    public:
      sref_back() : sback_(0) { }
      ~sref_back() { assert(sback_ == 0); }

      svector<const T2*>* back_list() const;

    private:
      void desert_(sref<T1,T2>*);
      sref<T1,T2>*sback_;
};

template <class T1, class T2> class sref {

      friend class sref_back<T1,T2>;

    public:
      sref(T1*d) : dest_(d) { insert_(); }
      virtual ~sref() { dest_->desert_(this); }

      T1*fore_ptr() { return dest_; }
      const T1*fore_ptr() const { return dest_; }

    private:
      T1*dest_;
      sref<T1,T2>*next_;

      void insert_()
	    { if (dest_->sback_ == 0) {
		    next_ = this;
		    dest_->sback_ = this;
	      } else {
		    next_ = dest_->sback_->next_;
		    dest_->sback_->next_ = this;
	      }
	    }

};

template <class T1,class T2>
svector<const T2*>* sref_back<T1,T2>::back_list() const
{
      if (sback_ == 0) return 0;
      unsigned cnt = 1;
      sref<T1,T2>*cur = sback_->next_;
      while (cur != sback_) {
	    cnt += 1;
	    cur = cur->next_;
      }

      svector<const T2*>* result = new svector<const T2*>(cnt);
      (*result)[0] = dynamic_cast<const T2*>(sback_);
      cur = sback_->next_;
      cnt = 1;
      while (cur != sback_) {
	    (*result)[cnt] = dynamic_cast<const T2*>(cur);
	    cnt += 1;
	    cur = cur->next_;
      }
      return result;
}

template <class T1, class T2> void sref_back<T1,T2>::desert_(sref<T1,T2>*item)
{
      if (item == sback_)
	    sback_ = item->next_;

      if (item == sback_) {
	    sback_ = 0;

      } else {
	    sref<T1,T2>*cur = sback_;
	    while (cur->next_ != item) {
		  assert(cur->next_);
		  cur = cur->next_;
	    }

	    cur->next_ = item->next_;
      }
}

/*
 * $Log: sref.h,v $
 * Revision 1.2  1999/07/18 05:52:47  steve
 *  xnfsyn generates DFF objects for XNF output, and
 *  properly rewrites the Design netlist in the process.
 *
 * Revision 1.1  1999/05/01 02:57:53  steve
 *  Handle much more complex event expressions.
 *
 */
#endif
