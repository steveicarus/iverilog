/*
 *  VHDL abstract syntax elements.
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

#include "vhdl_element.hh"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <typeinfo>
#include <iostream>
#include <sstream>

using namespace std;

static const int VHDL_INDENT = 2;  // Spaces to indent

int indent(int level)
{
   return level + VHDL_INDENT;
}

std::string nl_string(int level)
{
   std::ostringstream ss;
   newline(ss, level);
   return ss.str();
}

/*
 * Emit a newline and indent to the correct level.
 */
void newline(std::ostream &of, int level)
{
   of << std::endl;
   while (level--)
      of << ' ';
}

void blank_line(std::ostream &of, int level)
{
   of << std::endl;
   newline(of, level);
}

// The array of all vhdl_elements allocated so we can quickly
// clean them up just before the code generator exits
vector<vhdl_element*> vhdl_element::allocated_;

// Just a counter of total bytes allocated for statistics
size_t vhdl_element::total_alloc_(0);

void vhdl_element::set_comment(const std::string&comment)
{
   comment_ = comment;
}

/*
 * Draw the comment for any element. The comment is either on
 * a line before the element (end_of_line is false) or at the
 * end of the line containing the element (end_of_line is true).
 */
void vhdl_element::emit_comment(std::ostream &of, int level,
                                bool end_of_line) const
{
   if (! comment_.empty()) {
      if (end_of_line)
         of << "  -- " << comment_;
      else {
         // Comment may contain embedded newlines
         of << "-- ";
         for (string::const_iterator it = comment_.begin();
              it != comment_.end(); ++it) {
            if (*it == '\n') {
               newline(of, level);
               of << "-- ";
            }
            else
               of << *it;
         }
         newline(of, level);
      }
   }
}

void vhdl_element::print() const
{
   emit(std::cout, 0);
   std::cout << std::endl;
}

// Trap allocations of vhdl_element subclasses.
// This records the pointer allocated in a static field of vhdl_element
// so we can delete it just before the code generator exits.
void* vhdl_element::operator new(size_t size)
{
   // Let the default new handle the allocation
   void* ptr = ::operator new(size);

   // Remember this element so we can delete it later
   vhdl_element* elem = static_cast<vhdl_element*>(ptr);
   allocated_.push_back(elem);

   total_alloc_ += size;

   return ptr;
}

// Explicitly delete a vhdl_element object.
// This just sets the corresponding pointer in vhdl_element::allocated_
// to NULL (since it's safe to delete a NULL pointer).
void vhdl_element::operator delete(void* ptr)
{
   // Let the default delete handle the deallocation
   ::operator delete(ptr);

   // Remember that we've already deleted this pointer so we don't
   // delete it again in the call to free_all_objects
   vector<vhdl_element*>::iterator it =
      find(allocated_.begin(), allocated_.end(), static_cast<vhdl_element*>(ptr));

   if (it != allocated_.end()) {
      *it = NULL;   // It's safe to delete a NULL pointer and much cheaper
                    // than removing an element from the middle of a vector
   }
   else {
      // This shouldn't really happen but it's harmless
      cerr << "??? vhdl_element::operator delete called on an object not "
           << "allocated by vhdl_element::operator new" << endl;
   }
}

// Return the total number of bytes our custom operator new has seen.
size_t vhdl_element::total_allocated()
{
   return total_alloc_;
}

// Free every object derived from vhdl_element that has not yet been
// explicitly deallocated.
// Any pointers to vhdl_elements will be invalid after this call!
// Returns the number of objects freed.
int vhdl_element::free_all_objects()
{
   for (vector<vhdl_element*>::iterator it = allocated_.begin();
        it != allocated_.end(); ++it) {
      if (*it)
         ::operator delete(*it);  // Explicitly use the default delete
   }

   int freed = allocated_.size();

   // Just in case we want to allocated any more vhdl_element objects
   allocated_.clear();

   return freed;
}
