#ifndef IVL_vvp_darray_H
#define IVL_vvp_darray_H
/*
 * Copyright (c) 2012-2015 Stephen Williams (steve@icarus.com)
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
# include  "vvp_net.h"
# include  <list>
# include  <string>
# include  <vector>

class vvp_darray : public vvp_object {

    public:
      inline vvp_darray() { }
      virtual ~vvp_darray();

      virtual size_t get_size(void) const =0;

      virtual void set_word(unsigned adr, const vvp_vector4_t&value);
      virtual void get_word(unsigned adr, vvp_vector4_t&value);

      virtual void set_word(unsigned adr, double value);
      virtual void get_word(unsigned adr, double&value);

      virtual void set_word(unsigned adr, const std::string&value);
      virtual void get_word(unsigned adr, std::string&value);

      virtual void set_word(unsigned adr, const vvp_object_t&value);
      virtual void get_word(unsigned adr, vvp_object_t&value);
};

template <class TYPE> class vvp_darray_atom : public vvp_darray {

    public:
      inline vvp_darray_atom(size_t siz) : array_(siz) { }
      ~vvp_darray_atom();

      size_t get_size(void) const;
      void set_word(unsigned adr, const vvp_vector4_t&value);
      void get_word(unsigned adr, vvp_vector4_t&value);

    private:
      std::vector<TYPE> array_;
};

class vvp_darray_vec4 : public vvp_darray {

    public:
      inline vvp_darray_vec4(size_t siz, unsigned word_wid) :
                             array_(siz), word_wid_(word_wid) { }
      ~vvp_darray_vec4();

      size_t get_size(void) const;
      void set_word(unsigned adr, const vvp_vector4_t&value);
      void get_word(unsigned adr, vvp_vector4_t&value);

    private:
      std::vector<vvp_vector4_t> array_;
      unsigned word_wid_;
};

class vvp_darray_vec2 : public vvp_darray {

    public:
      inline vvp_darray_vec2(size_t siz, unsigned word_wid) :
                             array_(siz), word_wid_(word_wid) { }
      ~vvp_darray_vec2();

      size_t get_size(void) const;
      void set_word(unsigned adr, const vvp_vector4_t&value);
      void get_word(unsigned adr, vvp_vector4_t&value);

    private:
      std::vector<vvp_vector2_t> array_;
      unsigned word_wid_;
};

class vvp_darray_real : public vvp_darray {

    public:
      inline vvp_darray_real(size_t siz) : array_(siz) { }
      ~vvp_darray_real();

      size_t get_size(void) const;
      void set_word(unsigned adr, double value);
      void get_word(unsigned adr, double&value);

    private:
      std::vector<double> array_;
};

class vvp_darray_string : public vvp_darray {

    public:
      inline vvp_darray_string(size_t siz) : array_(siz) { }
      ~vvp_darray_string();

      size_t get_size(void) const;
      void set_word(unsigned adr, const std::string&value);
      void get_word(unsigned adr, std::string&value);

    private:
      std::vector<std::string> array_;
};

class vvp_darray_object : public vvp_darray {

    public:
      inline vvp_darray_object(size_t siz) : array_(siz) { }
      ~vvp_darray_object();

      size_t get_size(void) const;
      void set_word(unsigned adr, const vvp_object_t&value);
      void get_word(unsigned adr, vvp_object_t&value);

    private:
      std::vector<vvp_object_t> array_;
};

class vvp_queue : public vvp_darray {

    public:
      inline vvp_queue(void) { }
      ~vvp_queue();

      virtual void push_back(const vvp_vector4_t&value);
      virtual void push_front(const vvp_vector4_t&value);

      virtual void push_back(double value);
      virtual void push_front(double value);

      virtual void push_back(const std::string&value);
      virtual void push_front(const std::string&value);

      virtual void pop_back(void) =0;
      virtual void pop_front(void)=0;
};

class vvp_queue_vec4 : public vvp_queue {

    public:
      ~vvp_queue_vec4();

      size_t get_size(void) const;
      void set_word(unsigned adr, const vvp_vector4_t&value);
      void get_word(unsigned adr, vvp_vector4_t&value);
      void push_back(const vvp_vector4_t&value);
      void push_front(const vvp_vector4_t&value);
      void pop_back(void);
      void pop_front(void);

    private:
      std::list<vvp_vector4_t> array_;
};


class vvp_queue_string : public vvp_queue {

    public:
      ~vvp_queue_string();

      size_t get_size(void) const;
      void set_word(unsigned adr, const std::string&value);
      void get_word(unsigned adr, std::string&value);
      void push_back(const std::string&value);
	//void push_front(const std::string&value);
      void pop_back(void);
      void pop_front(void);

    private:
      std::list<std::string> array_;
};

#endif /* IVL_vvp_darray_H */
