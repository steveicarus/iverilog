/*
 * Copyright (c) 2026 Icarus UVM track
 */

# include  "vvp_aarray.h"
# include  <cassert>

using namespace std;

vvp_aarray::~vvp_aarray()
{
}

void vvp_aarray::set_word(const string&, const vvp_vector4_t&)
{
      assert(0);
}

void vvp_aarray::get_word(const string&, vvp_vector4_t&) const
{
      assert(0);
}

vvp_aarray_vec4::vvp_aarray_vec4(unsigned word_wid)
: word_wid_(word_wid)
{
}

vvp_aarray_vec4::~vvp_aarray_vec4()
{
}

size_t vvp_aarray_vec4::get_size() const
{
      return map_.size();
}

void vvp_aarray_vec4::clear()
{
      map_.clear();
}

bool vvp_aarray_vec4::exists(const string&key) const
{
      return map_.find(key) != map_.end();
}

void vvp_aarray_vec4::erase(const string&key)
{
      map_.erase(key);
}

void vvp_aarray_vec4::set_word(const string&key, const vvp_vector4_t&value)
{
      vvp_vector4_t tmp = value;
      if (tmp.size() != word_wid_)
	    tmp.resize(word_wid_);
      map_[key] = tmp;
}

void vvp_aarray_vec4::get_word(const string&key, vvp_vector4_t&value) const
{
      map<string,vvp_vector4_t>::const_iterator cur = map_.find(key);
      if (cur == map_.end()) {
	    value = vvp_vector4_t(word_wid_, BIT4_X);
	    return;
      }
      value = cur->second;
}

string vvp_aarray_vec4::key_at(size_t idx) const
{
      if (idx >= map_.size())
	    return string();
      map<string,vvp_vector4_t>::const_iterator cur = map_.begin();
      for (size_t i = 0; i < idx; i += 1)
	    ++cur;
      return cur->first;
}

void vvp_aarray_vec4::shallow_copy(const vvp_object*obj)
{
      const vvp_aarray_vec4*that = dynamic_cast<const vvp_aarray_vec4*>(obj);
      assert(that);
      map_ = that->map_;
      word_wid_ = that->word_wid_;
}

vvp_object* vvp_aarray_vec4::duplicate() const
{
      vvp_aarray_vec4*obj = new vvp_aarray_vec4(word_wid_);
      obj->map_ = map_;
      return obj;
}
