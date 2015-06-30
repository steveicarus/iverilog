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

      return VType::type_match(type_);
}
