/*
 * Copyright (c) 2012-2019 Stephen Williams (steve@icarus.com)
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

# include  "class_type.h"
# include  "compile.h"
# include  "vpi_priv.h"
# include  "config.h"
# include  <map>
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <cassert>

using namespace std;

/*
 * This class_property_t class is an abstract base class for
 * representing a property of an instance. The definition keeps and
 * array (of pointers) of these in order to define the the class.
 */
class class_property_t {
    public:
      inline class_property_t() { }
      virtual ~class_property_t() =0;
	// How much space does an instance of this property require?
      virtual size_t instance_size() const =0;

      void set_offset(size_t off) { offset_ = off; }

    public:
      virtual void construct(char*buf) const;
      virtual void destruct(char*buf) const;

      virtual void set_vec4(char*buf, const vvp_vector4_t&val);
      virtual void get_vec4(char*buf, vvp_vector4_t&val);

      virtual void set_real(char*buf, double val);
      virtual double get_real(char*buf);

      virtual void set_string(char*buf, const std::string&val);
      virtual string get_string(char*buf);

      virtual void set_object(char*buf, const vvp_object_t&val, uint64_t element);
      virtual void get_object(char*buf, vvp_object_t&val, uint64_t element);

	// Implement polymorphic shallow copy.
      virtual void copy(char*buf, char*src) = 0;

    protected:
      size_t offset_;
};

class_property_t::~class_property_t()
{
}

void class_property_t::construct(char*) const
{
}

void class_property_t::destruct(char*) const
{
}

void class_property_t::set_vec4(char*, const vvp_vector4_t&)
{
      assert(0);
}

void class_property_t::get_vec4(char*, vvp_vector4_t&)
{
      assert(0);
}

void class_property_t::set_real(char*, double)
{
      assert(0);
}

double class_property_t::get_real(char*)
{
      assert(0);
      return 0.0;
}

void class_property_t::set_string(char*, const string&)
{
      assert(0);
}

string class_property_t::get_string(char*)
{
      assert(0);
      return "";
}

void class_property_t::set_object(char*, const vvp_object_t&, uint64_t)
{
      assert(0);
}

void class_property_t::get_object(char*, vvp_object_t&, uint64_t)
{
      assert(0);
}

/*
 */
template <class T> class property_atom : public class_property_t {
    public:
      inline explicit property_atom(void) { }
      ~property_atom() { }

      size_t instance_size() const { return sizeof(T); }

    public:
      void construct(char*buf) const
      { T*tmp = reinterpret_cast<T*> (buf+offset_);
	*tmp = 0;
      }

      void set_vec4(char*buf, const vvp_vector4_t&val);
      void get_vec4(char*buf, vvp_vector4_t&val);

      void copy(char*dst, char*src);
};

class property_bit : public class_property_t {
    public:
      explicit inline property_bit(size_t wid): wid_(wid) { }
      ~property_bit() { }

      size_t instance_size() const { return sizeof(vvp_vector2_t); }

    public:
      void construct(char*buf) const
      { new (buf+offset_) vvp_vector2_t (0, wid_); }

      void destruct(char*buf) const
      { vvp_vector2_t*tmp = reinterpret_cast<vvp_vector2_t*>(buf+offset_);
	tmp->~vvp_vector2_t();
      }

      void set_vec4(char*buf, const vvp_vector4_t&val);
      void get_vec4(char*buf, vvp_vector4_t&val);

      void copy(char*dst, char*src);

    private:
      size_t wid_;
};

class property_logic : public class_property_t {
    public:
      explicit inline property_logic(size_t wid): wid_(wid) { }
      ~property_logic() { }

      size_t instance_size() const { return sizeof(vvp_vector4_t); }

    public:
      void construct(char*buf) const
      { new (buf+offset_) vvp_vector4_t (wid_); }

      void destruct(char*buf) const
      { vvp_vector4_t*tmp = reinterpret_cast<vvp_vector4_t*>(buf+offset_);
	tmp->~vvp_vector4_t();
      }

      void set_vec4(char*buf, const vvp_vector4_t&val);
      void get_vec4(char*buf, vvp_vector4_t&val);

      void copy(char*dst, char*src);

    private:
      size_t wid_;
};

template <class T> class property_real : public class_property_t {
    public:
      inline explicit property_real(void) { }
      ~property_real() { }

      size_t instance_size() const { return sizeof(T); }

    public:
      void construct(char*buf) const
      { T*tmp = reinterpret_cast<T*> (buf+offset_);
	*tmp = 0.0;
      }

      void set_real(char*buf, double val);
      double get_real(char*buf);

      void copy(char*dst, char*src);
};

class property_string : public class_property_t {
    public:
      inline explicit property_string(void) { }
      ~property_string() { }

      size_t instance_size() const { return sizeof(std::string); }

    public:
      void construct(char*buf) const
      { /* string*tmp = */ new (buf+offset_) string; }

      void destruct(char*buf) const
      { string*tmp = reinterpret_cast<string*> (buf+offset_);
	tmp->~string();
      }

      void set_string(char*buf, const string&);
      string get_string(char*buf);

      void copy(char*dst, char*src);
};

class property_object : public class_property_t {
    public:
      inline explicit property_object(uint64_t as): array_size_(as==0? 1 : as) { }
      ~property_object() { }

      size_t instance_size() const { return array_size_ * sizeof(vvp_object_t); }

    public:
      void construct(char*buf) const;

      void destruct(char*buf) const;

      void set_object(char*buf, const vvp_object_t&, uint64_t);
      void get_object(char*buf, vvp_object_t&, uint64_t);

      void copy(char*dst, char*src);

    private:
      size_t array_size_;
};

template <class T> void property_atom<T>::set_vec4(char*buf, const vvp_vector4_t&val)
{
      T*tmp = reinterpret_cast<T*> (buf+offset_);
      bool flag = vector4_to_value(val, *tmp, true, false);
      assert(flag);
}

template <class T> void property_atom<T>::get_vec4(char*buf, vvp_vector4_t&val)
{
      T*src = reinterpret_cast<T*> (buf+offset_);
      const size_t tmp_cnt = sizeof(T)<sizeof(unsigned long)
				       ? 1
				       : sizeof(T) / sizeof(unsigned long);
      unsigned long tmp[tmp_cnt];
      tmp[0] = src[0];

      for (size_t idx = 1 ; idx < tmp_cnt ; idx += 1)
	    tmp[idx] = src[0] >> idx * 8 * sizeof(tmp[0]);

      val.resize(8*sizeof(T));
      val.setarray(0, val.size(), tmp);
}

template <class T> void property_atom<T>::copy(char*dst, char*src)
{
      T*dst_obj = reinterpret_cast<T*> (dst+offset_);
      T*src_obj = reinterpret_cast<T*> (src+offset_);
      *dst_obj = *src_obj;
}

void property_bit::set_vec4(char*buf, const vvp_vector4_t&val)
{
      vvp_vector2_t*obj = reinterpret_cast<vvp_vector2_t*> (buf+offset_);
      *obj = val;
}

void property_bit::get_vec4(char*buf, vvp_vector4_t&val)
{
      vvp_vector2_t*obj = reinterpret_cast<vvp_vector2_t*> (buf+offset_);
      val = vector2_to_vector4(*obj, obj->size());
}

void property_bit::copy(char*dst, char*src)
{
      vvp_vector2_t*dst_obj = reinterpret_cast<vvp_vector2_t*> (dst+offset_);
      vvp_vector2_t*src_obj = reinterpret_cast<vvp_vector2_t*> (src+offset_);
      *dst_obj = *src_obj;
}

void property_logic::set_vec4(char*buf, const vvp_vector4_t&val)
{
      vvp_vector4_t*obj = reinterpret_cast<vvp_vector4_t*> (buf+offset_);
      *obj = val;
}

void property_logic::get_vec4(char*buf, vvp_vector4_t&val)
{
      vvp_vector4_t*obj = reinterpret_cast<vvp_vector4_t*> (buf+offset_);
      val = *obj;
}

void property_logic::copy(char*dst, char*src)
{
      vvp_vector4_t*dst_obj = reinterpret_cast<vvp_vector4_t*> (dst+offset_);
      vvp_vector4_t*src_obj = reinterpret_cast<vvp_vector4_t*> (src+offset_);
      *dst_obj = *src_obj;
}

template <class T> void property_real<T>::set_real(char*buf, double val)
{
      T*tmp = reinterpret_cast<T*>(buf+offset_);
      *tmp = val;
}

template <class T> double property_real<T>::get_real(char*buf)
{
      T*tmp = reinterpret_cast<T*>(buf+offset_);
      return *tmp;
}

template <class T> void property_real<T>::copy(char*dst, char*src)
{
      T*dst_obj = reinterpret_cast<T*> (dst+offset_);
      T*src_obj = reinterpret_cast<T*> (src+offset_);
      *dst_obj = *src_obj;
}

void property_string::set_string(char*buf, const string&val)
{
      string*tmp = reinterpret_cast<string*>(buf+offset_);
      *tmp = val;
}

string property_string::get_string(char*buf)
{
      string*tmp = reinterpret_cast<string*>(buf+offset_);
      return *tmp;
}

void property_string::copy(char*dst, char*src)
{
      string*dst_obj = reinterpret_cast<string*> (dst+offset_);
      string*src_obj = reinterpret_cast<string*> (src+offset_);
      *dst_obj = *src_obj;
}

void property_object::construct(char*buf) const
{
      for (size_t idx = 0 ; idx < array_size_ ; idx += 1)
	    new (buf+offset_ + idx*sizeof(vvp_object_t)) vvp_object_t;
}

void property_object::destruct(char*buf) const
{
      vvp_object_t*tmp = reinterpret_cast<vvp_object_t*> (buf+offset_);
      for (size_t idx = 0 ; idx < array_size_ ; idx += 1)
	    (tmp+idx)->~vvp_object_t();
}

void property_object::set_object(char*buf, const vvp_object_t&val, uint64_t idx)
{
      assert(idx < array_size_);
      vvp_object_t*tmp = reinterpret_cast<vvp_object_t*>(buf+offset_);
      tmp[idx] = val;
}

void property_object::get_object(char*buf, vvp_object_t&val, uint64_t idx)
{
      assert(idx < array_size_);
      vvp_object_t*tmp = reinterpret_cast<vvp_object_t*>(buf+offset_);
      val = tmp[idx];
}

void property_object::copy(char*dst, char*src)
{
      vvp_object_t*dst_obj = reinterpret_cast<vvp_object_t*>(dst+offset_);
      vvp_object_t*src_obj = reinterpret_cast<vvp_object_t*>(src+offset_);
      for (size_t idx = 0 ; idx < array_size_ ; idx += 1)
	    dst_obj[idx] = src_obj[idx];
}

/* **** */

class_type::class_type(const string&nam, size_t nprop)
: class_name_(nam), properties_(nprop)
{
      instance_size_ = 0;
}

class_type::~class_type()
{
      for (size_t idx = 0 ; idx < properties_.size() ; idx += 1)
	    delete properties_[idx].type;
}

void class_type::set_property(size_t idx, const string&name, const string&type, uint64_t array_size)
{
      assert(idx < properties_.size());
      properties_[idx].name = name;

      if (type == "b8")
	    properties_[idx].type = new property_atom<uint8_t>;
      else if (type == "b16")
	    properties_[idx].type = new property_atom<uint16_t>;
      else if (type == "b32")
	    properties_[idx].type = new property_atom<uint32_t>;
      else if (type == "b64")
	    properties_[idx].type = new property_atom<uint64_t>;
      else if (type == "sb8")
	    properties_[idx].type = new property_atom<int8_t>;
      else if (type == "sb16")
	    properties_[idx].type = new property_atom<int16_t>;
      else if (type == "sb32")
	    properties_[idx].type = new property_atom<int32_t>;
      else if (type == "sb64")
	    properties_[idx].type = new property_atom<int64_t>;
      else if (type == "r")
	    properties_[idx].type = new property_real<double>;
      else if (type == "S")
	    properties_[idx].type = new property_string;
      else if (type == "o")
	    properties_[idx].type = new property_object(array_size);
      else if (type[0] == 'b') {
	    size_t wid = strtoul(type.c_str()+1, 0, 0);
	    properties_[idx].type = new property_bit(wid);
      } else if (type[0] == 'L') {
	    size_t wid = strtoul(type.c_str()+1,0,0);
	    properties_[idx].type = new property_logic(wid);
      } else if (type[0] == 's' && type[1] == 'L') {
	    size_t wid = strtoul(type.c_str()+2,0,0);
	    properties_[idx].type = new property_logic(wid);
      } else {
	    properties_[idx].type = 0;
      }
}

void class_type::finish_setup(void)
{
      map<size_t, vector<size_t> > size_map;
	// Add up all the sizes to get a total instance size. This
	// figures out how much memory a complete instance will need.
      size_t accum = 0;
      for (size_t idx = 0 ; idx < properties_.size() ; idx += 1) {
	    assert(properties_[idx].type);
	    size_t instance_size = properties_[idx].type->instance_size();
	    accum += instance_size;
	    size_map[instance_size].push_back(idx);
      }

      instance_size_ = accum;

	// Now allocate the properties to offsets within an instance
	// space. Allocate the properties largest objects first so
	// that they are assured better alignment.
      accum = 0;
      for (map<size_t, vector<size_t> >::reverse_iterator cur = size_map.rbegin()
		 ; cur != size_map.rend() ; ++ cur) {
	    for (size_t idx = 0 ; idx < cur->second.size() ; idx += 1) {
		  size_t pid = cur->second[idx];
		  class_property_t*ptype = properties_[pid].type;
		  assert(ptype->instance_size() == cur->first);
		  ptype->set_offset(accum);
		  accum += cur->first;
	    }
      }
}

class_type::inst_t class_type::instance_new() const
{
      char*buf = new char [instance_size_];

      for (size_t idx = 0 ; idx < properties_.size() ; idx += 1)
	    properties_[idx].type->construct(buf);

      return reinterpret_cast<inst_t> (buf);
}

void class_type::instance_delete(class_type::inst_t obj) const
{
      char*buf = reinterpret_cast<char*> (obj);

      for (size_t idx = 0 ; idx < properties_.size() ; idx += 1)
	    properties_[idx].type->destruct(buf);

      delete[]buf;
}

void class_type::set_vec4(class_type::inst_t obj, size_t pid,
			  const vvp_vector4_t&val) const
{
      char*buf = reinterpret_cast<char*> (obj);
      assert(pid < properties_.size());
      properties_[pid].type->set_vec4(buf, val);
}

void class_type::get_vec4(class_type::inst_t obj, size_t pid,
			  vvp_vector4_t&val) const
{
      char*buf = reinterpret_cast<char*> (obj);
      assert(pid < properties_.size());
      properties_[pid].type->get_vec4(buf, val);
}

void class_type::set_real(class_type::inst_t obj, size_t pid,
			  double val) const
{
      char*buf = reinterpret_cast<char*> (obj);
      assert(pid < properties_.size());
      properties_[pid].type->set_real(buf, val);
}

double class_type::get_real(class_type::inst_t obj, size_t pid) const
{
      char*buf = reinterpret_cast<char*> (obj);
      assert(pid < properties_.size());
      return properties_[pid].type->get_real(buf);
}

void class_type::set_string(class_type::inst_t obj, size_t pid,
			    const string&val) const
{
      char*buf = reinterpret_cast<char*> (obj);
      assert(pid < properties_.size());
      properties_[pid].type->set_string(buf, val);
}

string class_type::get_string(class_type::inst_t obj, size_t pid) const
{
      char*buf = reinterpret_cast<char*> (obj);
      assert(pid < properties_.size());
      return properties_[pid].type->get_string(buf);
}

void class_type::set_object(class_type::inst_t obj, size_t pid,
			    const vvp_object_t&val, size_t idx) const
{
      char*buf = reinterpret_cast<char*> (obj);
      assert(pid < properties_.size());
      properties_[pid].type->set_object(buf, val, idx);
}

void class_type::get_object(class_type::inst_t obj, size_t pid,
			    vvp_object_t&val, size_t idx) const
{
      char*buf = reinterpret_cast<char*> (obj);
      assert(pid < properties_.size());
      properties_[pid].type->get_object(buf, val, idx);
}

void class_type::copy_property(class_type::inst_t dst, size_t pid, class_type::inst_t src) const
{
      char*dst_buf = reinterpret_cast<char*> (dst);
      char*src_buf = reinterpret_cast<char*> (src);

      assert(pid < properties_.size());

      properties_[pid].type->copy(dst_buf, src_buf);
}

int class_type::get_type_code(void) const
{
      return vpiClassDefn;
}

static class_type*compile_class = 0;

void compile_class_start(char*lab, char*nam, unsigned ntype)
{
      assert(compile_class == 0);
      compile_class = new class_type(nam, ntype);
      compile_vpi_symbol(lab, compile_class);
      free(lab);
      delete[]nam;
}

void compile_class_property(unsigned idx, char*nam, char*typ, uint64_t array_size)
{
      assert(compile_class);
      compile_class->set_property(idx, nam, typ, array_size);
      delete[]nam;
      delete[]typ;
}

void compile_class_done(void)
{
      __vpiScope*scope = vpip_peek_current_scope();
      assert(scope);
      assert(compile_class);
      compile_class->finish_setup();
      scope->classes[compile_class->class_name()] = compile_class;
      compile_class = 0;
}

#ifdef CHECK_WITH_VALGRIND
void class_def_delete(class_type *item)
{
      delete item;
}
#endif
