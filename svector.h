#ifndef __svector_H
#define __svector_H
/*
 * Copyright (c) 1999-2010 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  <string>
# include  <vector>
# include  <cassert>

/*
 * This is a way simplified vector class that cannot grow or shrink,
 * and is really only able to handle values. It is intended to be
 * lighter weight than the STL list class.
 */

template <class TYPE> class svector {

    public:
      explicit svector() : nitems_(0), items_(0) { }

      explicit svector(unsigned size) : nitems_(size), items_(new TYPE[size])
	    { for (unsigned idx = 0 ;  idx < size ;  idx += 1)
		  items_[idx] = 0;
	    }

      svector(const svector<TYPE>&that)
            : nitems_(that.nitems_), items_(new TYPE[nitems_])
	    { for (unsigned idx = 0 ;  idx < that.nitems_ ;  idx += 1)
		    items_[idx] = that[idx];
	    }

      svector(const svector<TYPE>&l, const svector<TYPE>&r)
            : nitems_(l.nitems_ + r.nitems_), items_(new TYPE[nitems_])
	    { for (unsigned idx = 0 ;  idx < l.nitems_ ;  idx += 1)
		    items_[idx] = l[idx];

	      for (unsigned idx = 0 ;  idx < r.nitems_ ;  idx += 1)
		    items_[l.nitems_+idx] = r[idx];
	    }

      svector(const svector<TYPE>&l, TYPE r)
            : nitems_(l.nitems_ + 1), items_(new TYPE[nitems_])
	    { for (unsigned idx = 0 ;  idx < l.nitems_ ;  idx += 1)
		    items_[idx] = l[idx];
	      items_[nitems_-1] = r;
	    }

      ~svector() { delete[]items_; }

      svector<TYPE>& operator= (const svector<TYPE>&that)
	    { if (&that == this) return *this;
	      delete[]items_;
	      nitems_ = that.nitems_;
	      items_ = new TYPE[nitems_];
	      for (unsigned idx = 0 ;  idx < nitems_ ;  idx += 1) {
		    items_[idx] = that.items_[idx];
	      }
	      return *this;
	    }

      unsigned count() const { return nitems_; }

      TYPE&operator[] (unsigned idx)
	    { assert(idx < nitems_);
	      return items_[idx];
	    }

      TYPE operator[] (unsigned idx) const
	    { assert(idx < nitems_);
	      return items_[idx];
	    }

    private:
      unsigned nitems_;
      TYPE* items_;

};

/*
 * Override the implementation of the above template for the string
 * type parameter. The initialization to nil works different here.
 */
template <> inline svector<std::string>::svector(unsigned size)
: nitems_(size), items_(new std::string[size])
{
}

/*
* This is a convenience function that converts an svector to a
* vector. This is to ease the transition from svector to vector so
* that the svector class can be gradually removed.
*/
template <class T> inline std::vector<T> vector_from_svector(const svector<T>&that)
{
      std::vector<T> res (that.count());
      for (unsigned idx = 0 ; idx < that.count() ; idx += 1)
	    res[idx] = that[idx];

      return res;
}

#endif
