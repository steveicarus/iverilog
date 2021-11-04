/*
 * Copyright CERN 2015-2021
 * @author Maciej Suminski (maciej.suminski@cern.ch)
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

#include "vtype.h"

class ActiveScope;

void emit_std_types(std::ostream&out);
void generate_global_types(ActiveScope*res);
bool is_global_type(perm_string type_name);
void delete_global_types();
const VTypeEnum*find_std_enum_name(perm_string name);

extern const VTypePrimitive primitive_BIT;
extern const VTypePrimitive primitive_INTEGER;
extern const VTypePrimitive primitive_NATURAL;
extern const VTypePrimitive primitive_REAL;
extern const VTypePrimitive primitive_STDLOGIC;
extern const VTypePrimitive primitive_TIME;
extern const VTypePrimitive primitive_TEXT;
extern const VTypePrimitive primitive_LINE;

extern VTypeDef type_BOOLEAN;
extern VTypeDef type_FILE_OPEN_KIND;
extern VTypeDef type_FILE_OPEN_STATUS;

extern const VTypeArray primitive_CHARACTER;
extern const VTypeArray primitive_BIT_VECTOR;
extern const VTypeArray primitive_BOOL_VECTOR;
extern const VTypeArray primitive_STDLOGIC_VECTOR;
extern const VTypeArray primitive_STRING;
extern const VTypeArray primitive_SIGNED;
extern const VTypeArray primitive_UNSIGNED;
