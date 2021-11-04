/*
 *  Generate code to convert between VHDL types.
 *
 *  Copyright (C) 2008-2021  Nick Gasson (nick@nickg.me.uk)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "vhdl_syntax.hh"

#include "vhdl_target.h"
#include "support.hh"

#include <cassert>
#include <iostream>

using namespace std;

vhdl_expr *vhdl_expr::cast(const vhdl_type *to)
{
#if 0
   std::cout << "Cast: from=" << type_->get_string()
             << " (" << type_->get_width() << ") "
             << " to=" << to->get_string() << " ("
             << to->get_width() << ")" << std::endl;
#endif

   // If this expression hasn't been given a type then
   // we can't generate any type conversion code
   if (NULL == type_)
      return this;

   if (to->get_name() == type_->get_name()) {
      if (to->get_width() == type_->get_width())
         return this;  // Identical
      else
         return resize(to->get_width());
   }
   else {
      switch (to->get_name()) {
      case VHDL_TYPE_BOOLEAN:
         return to_boolean();
      case VHDL_TYPE_INTEGER:
         return to_integer();
      case VHDL_TYPE_UNSIGNED:
      case VHDL_TYPE_SIGNED:
      case VHDL_TYPE_STD_LOGIC_VECTOR:
         return to_vector(to->get_name(), to->get_width());
      case VHDL_TYPE_STD_LOGIC:
         return to_std_logic();
      case VHDL_TYPE_STD_ULOGIC:
         return to_std_ulogic();
      case VHDL_TYPE_STRING:
         return to_string();
      default:
         assert(false);
      }
   }
   assert(false);
   return NULL;
}

/*
 * Generate code to cast an expression to a vector type (std_logic_vector,
 * signed, unsigned).
 */
vhdl_expr *vhdl_expr::to_vector(vhdl_type_name_t name, int w)
{
   if (type_->get_name() == VHDL_TYPE_STD_LOGIC) {
      vhdl_expr *others = w == 1 ? NULL : new vhdl_const_bit('0');
      vhdl_bit_spec_expr *bs =
         new vhdl_bit_spec_expr(new vhdl_type(name, w - 1, 0), others);
      bs->add_bit(0, this);

      return bs;
   }
   else {
      // We have to cast the expression before resizing or the
      // wrong sign bit may be extended (i.e. when casting between
      // signed/unsigned *and* resizing)
      vhdl_type *t = new vhdl_type(name, w - 1, 0);
      vhdl_fcall *conv = new vhdl_fcall(t->get_string().c_str(), t);
      conv->add_expr(this);

      if (w != type_->get_width())
         return conv->resize(w);
      else
         return conv;
   }
}

/*
 * Convert a generic expression to an Integer.
 */
vhdl_expr *vhdl_expr::to_integer()
{
   vhdl_fcall *conv;
   if (type_->get_name() == VHDL_TYPE_STD_LOGIC) {
      require_support_function(SF_LOGIC_TO_INTEGER);
      conv = new vhdl_fcall(support_function::function_name(SF_LOGIC_TO_INTEGER),
                            vhdl_type::integer());
   }
   else
      conv = new vhdl_fcall("To_Integer", vhdl_type::integer());

   conv->add_expr(this);

   return conv;
}

vhdl_expr *vhdl_expr::to_string()
{
   bool numeric = type_->get_name() == VHDL_TYPE_UNSIGNED
      || type_->get_name() == VHDL_TYPE_SIGNED;

   if (numeric) {
      vhdl_fcall *image = new vhdl_fcall("integer'image", vhdl_type::string());
      image->add_expr(this->cast(vhdl_type::integer()));
      return image;
   }
   else {
      // Assume type'image exists
      vhdl_fcall *image = new vhdl_fcall(type_->get_string() + "'image",
                                         vhdl_type::string());
      image->add_expr(this);
      return image;
   }
}

/*
 * Convert a generic expression to a Boolean.
 */
vhdl_expr *vhdl_expr::to_boolean()
{
   if (type_->get_name() == VHDL_TYPE_STD_LOGIC) {
      // '1' is true all else are false
      vhdl_const_bit *one = new vhdl_const_bit('1');
      return new vhdl_binop_expr
         (this, VHDL_BINOP_EQ, one, vhdl_type::boolean());
   }
   else if (type_->get_name() == VHDL_TYPE_UNSIGNED) {
      // Need to use a support function for this conversion
      require_support_function(SF_UNSIGNED_TO_BOOLEAN);

      vhdl_fcall *conv =
         new vhdl_fcall(support_function::function_name(SF_UNSIGNED_TO_BOOLEAN),
                        vhdl_type::boolean());
      conv->add_expr(this);
      return conv;
   }
   else if (type_->get_name() == VHDL_TYPE_SIGNED) {
      require_support_function(SF_SIGNED_TO_BOOLEAN);

      vhdl_fcall *conv =
         new vhdl_fcall(support_function::function_name(SF_SIGNED_TO_BOOLEAN),
                        vhdl_type::boolean());
      conv->add_expr(this);
      return conv;
   }
   assert(false);
   return NULL;
}

/*
 * Generate code to convert and expression to std_logic.
 */
vhdl_expr *vhdl_expr::to_std_logic()
{
   if (type_->get_name() == VHDL_TYPE_BOOLEAN) {
      require_support_function(SF_BOOLEAN_TO_LOGIC);

      vhdl_fcall *ah =
         new vhdl_fcall(support_function::function_name(SF_BOOLEAN_TO_LOGIC),
                        vhdl_type::std_logic());
      ah->add_expr(this);

      return ah;
   }
   else if (type_->get_name() == VHDL_TYPE_SIGNED) {
      require_support_function(SF_SIGNED_TO_LOGIC);

      vhdl_fcall *ah =
         new vhdl_fcall(support_function::function_name(SF_SIGNED_TO_LOGIC),
                        vhdl_type::std_logic());
      ah->add_expr(this);

      return ah;
   }
   else if (type_->get_name() == VHDL_TYPE_UNSIGNED) {
      require_support_function(SF_UNSIGNED_TO_LOGIC);

      vhdl_fcall *ah =
         new vhdl_fcall(support_function::function_name(SF_UNSIGNED_TO_LOGIC),
                        vhdl_type::std_logic());
      ah->add_expr(this);

      return ah;
   }
   assert(false);
   return NULL;
}

vhdl_expr *vhdl_expr::to_std_ulogic()
{
   if (type_->get_name() == VHDL_TYPE_STD_LOGIC) {
      vhdl_fcall *f = new vhdl_fcall("std_logic", vhdl_type::std_logic());
      f->add_expr(this);
      return f;
   }
   else
      assert(false);
   return NULL;
}

/*
 * Change the width of a signed/unsigned type.
 */
vhdl_expr *vhdl_expr::resize(int newwidth)
{
   vhdl_type *rtype;
   assert(type_);
   if (type_->get_name() == VHDL_TYPE_SIGNED)
      rtype = vhdl_type::nsigned(newwidth);
   else if (type_->get_name() == VHDL_TYPE_UNSIGNED)
      rtype = vhdl_type::nunsigned(newwidth);
   else if (type_->get_name() == VHDL_TYPE_STD_LOGIC) {
      // Pad it with zeros
      vhdl_expr* zeros = new vhdl_const_bits(string(newwidth - 1, '0').c_str(),
                                             newwidth - 1, false, true);

      vhdl_binop_expr* concat =
         new vhdl_binop_expr(zeros, VHDL_BINOP_CONCAT, this,
                             vhdl_type::nunsigned(newwidth));
      return concat;
   }
   else
      return this;   // Doesn't make sense to resize non-vector type

   vhdl_fcall *resizef = new vhdl_fcall("Resize", rtype);
   resizef->add_expr(this);
   resizef->add_expr(new vhdl_const_int(newwidth));

   return resizef;
}

vhdl_expr *vhdl_const_int::to_vector(vhdl_type_name_t name, int w)
{
   if (name == VHDL_TYPE_SIGNED || name == VHDL_TYPE_UNSIGNED) {

      const char *fname = name == VHDL_TYPE_SIGNED
         ? "To_Signed" : "To_Unsigned";
      vhdl_fcall *conv = new vhdl_fcall(fname, new vhdl_type(name, w - 1, 0));
      conv->add_expr(this);
      conv->add_expr(new vhdl_const_int(w));

      return conv;
   }
   else
      return vhdl_expr::to_vector(name, w);
}

int64_t vhdl_const_bits::bits_to_int() const
{
   char msb = value_[value_.size() - 1];
   int64_t result = 0, bit;
   for (int i = sizeof(int64_t)*8 - 1; i >= 0; i--) {
      if (i > (int)value_.size() - 1)
         bit = (msb == '1' && signed_) ? 1 : 0;
      else
         bit = value_[i] == '1' ? 1 : 0;
      result = (result << 1) | bit;
   }

   return result;
}

vhdl_expr *vhdl_const_bits::to_std_logic()
{
   // VHDL won't let us cast directly between a vector and
   // a scalar type
   // But we don't need to here as we have the bits available

   // Take the least significant bit
   char lsb = value_[0];

   return new vhdl_const_bit(lsb);
}

char vhdl_const_bits::sign_bit() const
{
   return signed_ ? value_[value_.length()-1] : '0';
}

vhdl_expr *vhdl_const_bits::to_vector(vhdl_type_name_t name, int w)
{
   if (name == VHDL_TYPE_STD_LOGIC_VECTOR) {
      // Don't need to do anything
      return this;
   }
   else if (name == VHDL_TYPE_SIGNED || name == VHDL_TYPE_UNSIGNED) {
      // Extend with sign bit
      value_.resize(w, sign_bit());
      return this;
   }
   assert(false);
   return NULL;
}

vhdl_expr *vhdl_const_bits::to_integer()
{
   return new vhdl_const_int(bits_to_int());
}

vhdl_expr *vhdl_const_bits::resize(int w)
{
   // Rather than generating a call to Resize, when can just sign-extend
   // the bits here. As well as looking better, this avoids any ambiguity
   // between which of the signed/unsigned versions of Resize to use.

   value_.resize(w, sign_bit());
   return this;
}

vhdl_expr *vhdl_const_bit::to_integer()
{
   return new vhdl_const_int(bit_ == '1' ? 1 : 0);
}

vhdl_expr *vhdl_const_bit::to_boolean()
{
   return new vhdl_const_bool(bit_ == '1');
}

vhdl_expr *vhdl_const_bit::to_std_ulogic()
{
   return this;
}

vhdl_expr *vhdl_const_bit::to_vector(vhdl_type_name_t name, int w)
{
   // Zero-extend this bit to the correct width
   return (new vhdl_const_bits(&bit_, 1, name == VHDL_TYPE_SIGNED))->resize(w);
}
