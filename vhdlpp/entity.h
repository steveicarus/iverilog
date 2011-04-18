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
# include  <list>
# include  <vector>
# include  <iostream>
# include  "StringHeap.h"
# include  "LineInfo.h"
# include  "vtype.h"

typedef enum { PORT_NONE=0, PORT_IN, PORT_OUT } port_mode_t;

class Architecture;

class InterfacePort : public LineInfo {
    public:
	// Port direction from the source code.
      port_mode_t mode;
	// Name of the port from the source code
      perm_string name;
	// Name of interface type as given in the source code.
      const VType*type;
};

/*
 * The ComponentBase class represents the base entity
 * declaration. When used as is, then this represents a forward
 * declaration of an entity. Elaboration will match it to a proper
 * entity. Or this can be the base class for a full-out Entity.
 */
class ComponentBase : public LineInfo {

    public:
      ComponentBase(perm_string name);
      ~ComponentBase();

	// Entities have names.
      perm_string get_name() const { return name_; }
      const InterfacePort* find_port(perm_string by_name) const;

	// Declare the ports for the entity. The parser calls this
	// method with a list of interface elements that were parsed
	// for the entity. This method collects those entities, and
	// empties the list in the process.
      void set_interface(std::list<InterfacePort*>*ports);

    public:
      void dump_ports(ostream&out, int indent = 0) const;

    protected:
	// This is really only used by the Entity derived class.
      const std::vector<InterfacePort*>&get_ports() { return ports_; }

    private:
      perm_string name_;
      std::vector<InterfacePort*> ports_;
};

/*
 * Entities are fully declared components.
 */
class Entity : public ComponentBase {

    public:
      Entity(perm_string name);
      ~Entity();

	// bind an architecture to the entity, and return the
	// Architecture that was bound. If there was a previous
	// architecture with the same name bound, then do not replace
	// the architecture and instead return the old
	// value. The caller can tell that the bind worked if the
	// returned pointer is the same as the passed pointer.
      Architecture* add_architecture(Architecture*);

      int elaborate();
      int emit(ostream&out);

      void dump(ostream&out, int indent = 0) const;

    private:
      std::map<perm_string,Architecture*>arch_;
      Architecture*bind_arch_;

      map<perm_string,VType::decl_t> declarations_;

      int elaborate_ports_(void);
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

extern int emit_entities(void);

/*
 * Use this function to dump a description of the design entities to a
 * file. This is for debug, not for any useful purpose.
 */
extern void dump_design_entities(ostream&file);

#endif
