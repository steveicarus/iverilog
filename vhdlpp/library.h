#ifndef IVL_library_H
#define IVL_library_H
/*
 * Copyright (c) 2011-2021 Stephen Williams (steve@icarus.com)
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

#include <list>

class SubprogramHeader;
class VType;

void library_set_work_path(const char*work_path);
void library_add_directory(const char*directory);

int elaborate_libraries(void);
int emit_packages(void);

SubprogramHeader*library_match_subprogram(perm_string name, const std::list<const VType*>*params);

#endif /* IVL_library_H */
