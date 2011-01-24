#ifndef __entity_H
#define __entity_H
/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  <map>
# include  <vector>
# include  "StringHeap.h"
# include  "LineInfo.h"

typedef enum { PORT_NONE=0, PORT_IN, PORT_OUT } port_mode_t;

class Architecture;

class InterfacePort : public LineInfo {
    public:
	// Port direction from the source code.
      port_mode_t mode;
	// Name of the port from the source code
      perm_string name;
	// Name of interface type as given in the source code.
      perm_string type_name;
};

class Entity : public LineInfo {

    public:
      Entity(perm_string name);
      ~Entity();

	// Entities have names.
      perm_string get_name() const { return name_; }

	// bind an architecture to the entity, and return the
	// Architecture that was bound. If there was a previous
	// architecture with the same name bound, then do not replace
	// the architecture and instead return the old
	// value. The caller can tell that the bind worked if the
	// returned pointer is the same as the passed pointer.
      Architecture* add_architecture(Architecture*);

      int elaborate();

      void dump(ostream&out) const;

    public:
      std::vector<InterfacePort*> ports;

    private:
      perm_string name_;

      std::map<perm_string,Architecture*>arch_;
};

/*
 * As the parser parses entities, it puts them into this map. It uses
 * a map because sometimes it needs to look back at an entity by name.
 */
extern std::map<perm_string,Entity*> design_entities;

/*
 * Elaborate the collected entities, and return the number of
 * elaboration errors.
 */
extern int elaborate_entities(void);

/*
 * Use this function to dump a description of the design entities to a
 * file. This is for debug, not for any useful purpose.
 */
extern void dump_design_entities(const char*path);

#endif
