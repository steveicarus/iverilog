#ifndef __vvp_object_H
#define __vvp_object_H
/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  <stdlib.h>
# include  <string>
# include  <vector>

class vvp_vector4_t;
class vvp_object_t;

/*
 * A vvp_object is a garbage collected object such as a darray or
 * class object. The vvp_object class is a virtual base class and not
 * generally used directly. Instead, use the vvp_object_t object as a
 * smart pointer. This makes garbage collection automatic.
 */
class vvp_object {
    public:
      inline vvp_object() { total_active_cnt_ += 1; }
      virtual ~vvp_object() =0;

      static void cleanup(void);

    private:
      friend class vvp_object_t;
      int ref_cnt_;

      static int total_active_cnt_;
};

class vvp_object_t {
    public:
      inline vvp_object_t() : ref_(0) { }
      vvp_object_t(const vvp_object_t&that);
      ~vvp_object_t();

      vvp_object_t& operator = (const vvp_object_t&that);
      vvp_object_t& operator = (class vvp_object*that);

      void reset(vvp_object*tgt = 0);

      bool test_nil() const { return ref_ == 0; }
      inline bool operator == (const vvp_object_t&that) const
          { return ref_ == that.ref_; }
      inline bool operator != (const vvp_object_t&that) const
          { return ref_ != that.ref_; }

      template <class T> T*peek(void) const;

    private:
      class vvp_object*ref_;
};

inline vvp_object_t::vvp_object_t(const vvp_object_t&that)
{
      ref_ = that.ref_;
      if (ref_) ref_->ref_cnt_ += 1;
}

inline vvp_object_t::~vvp_object_t()
{
      reset(0);
}

/*
 * This is the workhorse of the vvp_object_t class. It manages the
 * pointer to the referenced object.
 */
inline void vvp_object_t::reset(class vvp_object*tgt)
{
      if (tgt) tgt->ref_cnt_ += 1;
      if (ref_) {
	    ref_->ref_cnt_ -= 1;
	    if (ref_->ref_cnt_ <= 0) delete ref_;
	    ref_ = 0;
      }
      ref_ = tgt;
}

inline vvp_object_t& vvp_object_t::operator = (const vvp_object_t&that)
{
      if (this == &that) return *this;
      reset(that.ref_);
      return *this;
}

inline vvp_object_t& vvp_object_t::operator = (class vvp_object*that)
{
      reset(that);
      return *this;
}

/*
 * This peeks at the actual pointer value in the form of a derived
 * class. It uses dynamic_cast<>() to convert the pointer to the
 * desired type.
 *
 * NOTE: The vvp_object_t object retains ownership of the pointer!
 */
template <class T> inline T*vvp_object_t::peek(void) const
{
      return dynamic_cast<T*> (ref_);
}

class vvp_darray : public vvp_object {

    public:
      inline vvp_darray(size_t siz) : size_(siz) { }
      virtual ~vvp_darray();

      inline size_t get_size(void) const { return size_; }

      virtual void set_word(unsigned adr, const vvp_vector4_t&value);
      virtual void get_word(unsigned adr, vvp_vector4_t&value);

      virtual void set_word(unsigned adr, double value);
      virtual void get_word(unsigned adr, double&value);

      virtual void set_word(unsigned adr, const std::string&value);
      virtual void get_word(unsigned adr, std::string&value);

    private:
      size_t size_;
};

template <class TYPE> class vvp_darray_atom : public vvp_darray {

    public:
      inline vvp_darray_atom(size_t siz) : vvp_darray(siz), array_(siz) { }
      ~vvp_darray_atom();

      void set_word(unsigned adr, const vvp_vector4_t&value);
      void get_word(unsigned adr, vvp_vector4_t&value);

    private:
      std::vector<TYPE> array_;
};

class vvp_darray_real : public vvp_darray {

    public:
      inline vvp_darray_real(size_t siz) : vvp_darray(siz), array_(siz) { }
      ~vvp_darray_real();

      void set_word(unsigned adr, double value);
      void get_word(unsigned adr, double&value);

    private:
      std::vector<double> array_;
};

class vvp_darray_string : public vvp_darray {

    public:
      inline vvp_darray_string(size_t siz) : vvp_darray(siz), array_(siz) { }
      ~vvp_darray_string();

      void set_word(unsigned adr, const std::string&value);
      void get_word(unsigned adr, std::string&value);

    private:
      std::vector<std::string> array_;
};

#endif
