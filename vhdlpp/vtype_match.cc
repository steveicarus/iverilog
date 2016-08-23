/*
 * Copyright (c) 2013 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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

# include  "vtype.h"

bool VType::type_match(const VType*that) const
{
      if(this == that)
          return true;

      if(const VTypeDef*tdef = dynamic_cast<const VTypeDef*>(that)) {
          if(type_match(tdef->peek_definition()))
              return true;
      }

      return false;
}

bool VTypeDef::type_match(const VType*that) const
{
      if(VType::type_match(that))
          return true;

      return type_->type_match(that);
}

bool VTypePrimitive::type_match(const VType*that) const
{
      if(VType::type_match(that))
          return true;

      if(const VTypePrimitive*prim = dynamic_cast<const VTypePrimitive*>(that)) {
          // TODO it is not always true, but works for many cases
          type_t that_type = prim->type();
          return ((type_ == NATURAL || type_ == INTEGER) &&
              (that_type == NATURAL || that_type == INTEGER));
      }

      if(const VTypeRangeConst*range = dynamic_cast<const VTypeRangeConst*>(that)) {
           if (type_ == INTEGER)
               return true;
           if (type_ == NATURAL && range->start() >= 0 && range->end() >= 0)
               return true;
      }

      return false;
}

bool VTypeArray::type_match(const VType*that) const
{
      if(VType::type_match(that))
          return true;

      // Check if both arrays are of the same size
      if(const VTypeArray*arr = dynamic_cast<const VTypeArray*>(that)) {
          const VTypeArray*this_parent = this;
          while(const VTypeArray*tmp = this_parent->get_parent_type())
              this_parent = tmp;

          const VTypeArray*that_parent = arr;
          while(const VTypeArray*tmp = that_parent->get_parent_type())
              that_parent = tmp;

          if(this_parent != that_parent)
              return false;

          int this_width = get_width(NULL);
          int that_width = arr->get_width(NULL);

          // Either one of the sizes is undefined, or both are the same size
          if(this_width > 0 && that_width > 0 && this_width != that_width)
              return false;

          return true;
      }

      return false;
}

bool VTypeRange::type_match(const VType*that) const
{
      if(VType::type_match(that))
          return true;

      if(base_->type_match(that))
          return true;

      return false;
}
