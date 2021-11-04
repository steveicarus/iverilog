#ifndef IVL_vvp_darray_H
#define IVL_vvp_darray_H
/*
 * Copyright (c) 2012-2021 Stephen Williams (steve@icarus.com)
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
# include  <deque>
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

      virtual vvp_vector4_t get_bitstream(bool as_vec4);
};

template <class TYPE> class vvp_darray_atom : public vvp_darray {

    public:
      explicit inline vvp_darray_atom(size_t siz) : array_(siz) { }
      ~vvp_darray_atom();

      size_t get_size(void) const;
      void set_word(unsigned adr, const vvp_vector4_t&value);
      void get_word(unsigned adr, vvp_vector4_t&value);
      void shallow_copy(const vvp_object*obj);
      vvp_object* duplicate(void) const;
      vvp_vector4_t get_bitstream(bool as_vec4);

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
      void shallow_copy(const vvp_object*obj);
      vvp_object* duplicate(void) const;
      vvp_vector4_t get_bitstream(bool as_vec4);

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
      void shallow_copy(const vvp_object*obj);
      vvp_vector4_t get_bitstream(bool as_vec4);

    private:
      std::vector<vvp_vector2_t> array_;
      unsigned word_wid_;
};

class vvp_darray_real : public vvp_darray {

    public:
      explicit inline vvp_darray_real(size_t siz) : array_(siz) { }
      ~vvp_darray_real();

      size_t get_size(void) const;
      void set_word(unsigned adr, double value);
      void get_word(unsigned adr, double&value);
      void shallow_copy(const vvp_object*obj);
      vvp_object* duplicate(void) const;
      vvp_vector4_t get_bitstream(bool as_vec4);

    private:
      std::vector<double> array_;
};

class vvp_darray_string : public vvp_darray {

    public:
      explicit inline vvp_darray_string(size_t siz) : array_(siz) { }
      ~vvp_darray_string();

      size_t get_size(void) const;
      void set_word(unsigned adr, const std::string&value);
      void get_word(unsigned adr, std::string&value);
      void shallow_copy(const vvp_object*obj);
      vvp_object* duplicate(void) const;

    private:
      std::vector<std::string> array_;
};

class vvp_darray_object : public vvp_darray {

    public:
      explicit inline vvp_darray_object(size_t siz) : array_(siz) { }
      ~vvp_darray_object();

      size_t get_size(void) const;
      void set_word(unsigned adr, const vvp_object_t&value);
      void get_word(unsigned adr, vvp_object_t&value);
      void shallow_copy(const vvp_object*obj);
      //virtual vvp_object* duplicate(void) const;

    private:
      std::vector<vvp_object_t> array_;
};

class vvp_queue : public vvp_darray {

    public:
      inline vvp_queue(void) { }
      ~vvp_queue();

      virtual size_t get_size(void) const =0;
      virtual void copy_elems(vvp_object_t src, unsigned max_size);

      virtual void set_word_max(unsigned adr, const vvp_vector4_t&value, unsigned max_size);
      virtual void insert(unsigned idx, const vvp_vector4_t&value, unsigned max_size);
      virtual void push_back(const vvp_vector4_t&value, unsigned max_size);
      virtual void push_front(const vvp_vector4_t&value, unsigned max_size);

      virtual void set_word_max(unsigned adr, double value, unsigned max_size);
      virtual void insert(unsigned idx, double value, unsigned max_size);
      virtual void push_back(double value, unsigned max_size);
      virtual void push_front(double value, unsigned max_size);

      virtual void set_word_max(unsigned adr, const std::string&value, unsigned max_size);
      virtual void insert(unsigned idx, const std::string&value, unsigned max_size);
      virtual void push_back(const std::string&value, unsigned max_size);
      virtual void push_front(const std::string&value, unsigned max_size);

      virtual void pop_back(void) =0;
      virtual void pop_front(void)=0;
      virtual void erase(unsigned idx)=0;
      virtual void erase_tail(unsigned idx)=0;
};

class vvp_queue_real : public vvp_queue {

    public:
      ~vvp_queue_real();

      size_t get_size(void) const { return queue.size(); };
      void copy_elems(vvp_object_t src, unsigned max_size);
      void set_word_max(unsigned adr, double value, unsigned max_size);
      void set_word(unsigned adr, double value);
      void get_word(unsigned adr, double&value);
      void insert(unsigned idx, double value, unsigned max_size);
      void push_back(double value, unsigned max_size);
      void push_front(double value, unsigned max_size);
      void pop_back(void) { queue.pop_back(); };
      void pop_front(void) { queue.pop_front(); };
      void erase(unsigned idx);
      void erase_tail(unsigned idx);

    private:
      std::deque<double> queue;
};

class vvp_queue_string : public vvp_queue {

    public:
      ~vvp_queue_string();

      size_t get_size(void) const { return queue.size(); };
      void copy_elems(vvp_object_t src, unsigned max_size);
      void set_word_max(unsigned adr, const std::string&value, unsigned max_size);
      void set_word(unsigned adr, const std::string&value);
      void get_word(unsigned adr, std::string&value);
      void insert(unsigned idx, const std::string&value, unsigned max_size);
      void push_back(const std::string&value, unsigned max_size);
      void push_front(const std::string&value, unsigned max_size);
      void pop_back(void) { queue.pop_back(); };
      void pop_front(void) { queue.pop_front(); };
      void erase(unsigned idx);
      void erase_tail(unsigned idx);

    private:
      std::deque<std::string> queue;
};

class vvp_queue_vec4 : public vvp_queue {

    public:
      ~vvp_queue_vec4();

      size_t get_size(void) const { return queue.size(); };
      void copy_elems(vvp_object_t src, unsigned max_size);
      void set_word_max(unsigned adr, const vvp_vector4_t&value, unsigned max_size);
      void set_word(unsigned adr, const vvp_vector4_t&value);
      void get_word(unsigned adr, vvp_vector4_t&value);
      void insert(unsigned idx, const vvp_vector4_t&value, unsigned max_size);
      void push_back(const vvp_vector4_t&value, unsigned max_size);
      void push_front(const vvp_vector4_t&value, unsigned max_size);
      void pop_back(void) { queue.pop_back(); };
      void pop_front(void) { queue.pop_front(); };
      void erase(unsigned idx);
      void erase_tail(unsigned idx);

    private:
      std::deque<vvp_vector4_t> queue;
};

extern std::string get_fileline();

#endif /* IVL_vvp_darray_H */
