/*
 * Copyright (c) 2010-2011 Stephen Williams (steve@icarus.com)
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

# include  "compile.h"
# include  "enum_type.h"
# include  <iostream>
# include  <cassert>

struct enumconst_s {
      struct __vpiHandle base;
      const char*name;
      vvp_vector2_t val2;
      vvp_vector4_t val4;
};

static struct enumconst_s* enumconst_from_handle(vpiHandle obj)
{
      if (obj->vpi_type->type_code == vpiEnumConst)
	    return (struct enumconst_s*) obj;
      else
	    return 0;
}

struct __vpiEnumTypespec {
      struct __vpiHandle base;
      std::vector<enumconst_s> names;
      bool is_signed;
};

static struct __vpiEnumTypespec* vpip_enum_typespec_from_handle(vpiHandle obj)
{
      if (obj->vpi_type->type_code == vpiEnumTypespec)
	    return (struct __vpiEnumTypespec*) obj;

      return 0;
}

static int enum_type_get(int code, vpiHandle obj)
{
      struct __vpiEnumTypespec*ref = vpip_enum_typespec_from_handle(obj);
      assert(ref);

      switch (code) {
	  case vpiSize:
	    return ref->names.size();

	  case vpiSigned:
	    return ref->is_signed;

	  default:
	    fprintf(stderr, "vvp error: get %d not supported "
		    "by __vpiEnumTypespec\n", code);
	    assert(0);
	    return 0;
      }
}

static vpiHandle enum_type_iterate(int code, vpiHandle obj)
{
      struct __vpiEnumTypespec*ref = vpip_enum_typespec_from_handle(obj);
      assert(ref);

      if (code == vpiMember) {
	    vpiHandle*args = (vpiHandle*)
		  calloc(ref->names.size(), sizeof(vpiHandle*));
	    for (size_t idx = 0 ; idx < ref->names.size() ; idx += 1)
		  args[idx] = vpi_handle(&ref->names[idx]);

	    return vpip_make_iterator(ref->names.size(), args, true);
      }

      return 0;
}

static const struct __vpirt enum_type_rt = {
      vpiEnumTypespec,
      enum_type_get,
      0, //enum_type_get_str,
      0, //enum_type_get_value,
      0, //enum_type_put_value,
      0, //enum_type_handle,
      enum_type_iterate,
      0, //enum_type_index,
      0, //enum_type_free_object,
      0, //enum_type_get_delays,
      0, //enum_type_put_delays
};

static int enum_name_get(int code, vpiHandle obj)
{
      struct enumconst_s*ref = enumconst_from_handle(obj);
      assert(ref);

      switch (code) {
	  case vpiSize:
	    return ref->val4.size()? ref->val4.size() : ref->val2.size();
	  default:
	    return 0;
      }
}

static char* enum_name_get_str(int code, vpiHandle obj)
{
      struct enumconst_s*ref = enumconst_from_handle(obj);
      assert(ref);

      switch (code) {
	  case vpiName:
	    return const_cast<char*> (ref->name);
	  default:
	    return 0;
      }
}

static void enum_name_get_value(vpiHandle obj, p_vpi_value value)
{
      struct enumconst_s*ref = enumconst_from_handle(obj);
      assert(ref);

      if (ref->val4.size() > 0)
	    vpip_vec4_get_value(ref->val4, ref->val4.size(), false, value);
      else
	    vpip_vec2_get_value(ref->val2, ref->val2.size(), false, value);
}

static const struct __vpirt enum_name_rt = {
      vpiEnumConst,
      enum_name_get,
      enum_name_get_str,
      enum_name_get_value,
      0, //enum_name_put_value,
      0, //enum_name_handle,
      0, //enum_name_iterate,
      0, //enum_name_index,
      0, //enum_name_free_object,
      0, //enum_name_get_delays,
      0, //enum_name_put_delays
};

void compile_enum2_type(char*label, long width, bool signed_flag,
                        std::list<struct enum_name_s>*names)
{
      struct __vpiEnumTypespec*spec = new struct __vpiEnumTypespec;
      spec->base.vpi_type = &enum_type_rt;
      spec->names = std::vector<enumconst_s> (names->size());
      spec->is_signed = signed_flag;

      size_t idx = 0;
      for (list<struct enum_name_s>::iterator cur = names->begin()
		 ; cur != names->end() ;  ++cur, ++idx) {
	    assert(cur->val4 == 0);
	    spec->names[idx].base.vpi_type = &enum_name_rt;
	    spec->names[idx].name = cur->text;
	    spec->names[idx].val2 = vvp_vector2_t(cur->val2, width);
      }

      assert(idx == spec->names.size());
      compile_vpi_symbol(label, vpi_handle(spec));

      free(label);
      delete names;
}

void compile_enum4_type(char*label, long width, bool signed_flag,
                        std::list<struct enum_name_s>*names)
{
      struct __vpiEnumTypespec*spec = new struct __vpiEnumTypespec;
      spec->base.vpi_type = &enum_type_rt;
      spec->names = std::vector<enumconst_s> (names->size());
      spec->is_signed = signed_flag;

      size_t idx = 0;
      for (list<struct enum_name_s>::iterator cur = names->begin()
		 ; cur != names->end() ;  ++cur, ++idx) {
	    spec->names[idx].base.vpi_type = &enum_name_rt;
	    spec->names[idx].name = cur->text;
	    assert(cur->val4);
	    spec->names[idx].val4 = vector4_from_text(cur->val4, width);
	    free(cur->val4);
	    cur->val4 = 0;
      }

      assert(idx == spec->names.size());
      compile_vpi_symbol(label, vpi_handle(spec));

      free(label);
      delete names;
}
