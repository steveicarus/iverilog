#ifndef IVL_vvp_darray_H
#define IVL_vvp_darray_H
/*
 * Copyright (c) 2012-2014 Stephen Williams (steve@icarus.com)
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

# include  "vvp_object.h"
# include  <string>
# include  <vector>

class vvp_vector4_t;

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


#endif /* IVL_vvp_darray_H */
