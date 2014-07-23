#ifndef IVL_edif_H
#define IVL_edif_H
/*
 * Copyright (c) 2003-2014 Stephen Williams (steve@icarus.com)
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

# include  <stdio.h>
# include  <ivl_target.h>

/*
 * These types and functions support the task of generating and
 * writing out an EDIF 2 0 0 netlist. These functions work by
 * supporting the creation of an in-core netlist of the design, then
 * writing the netlist out all at once. The library manages cells with
 * ports, but does not otherwise interpret cells. They have no
 * contents.
 *
 * The general structure of netlist creation is as follows:
 *
 *   Create a netlist with edif_create(<name>);
 *     This creates an edif object that represents the design. The
 *     design is given a name, and that name becomes the name of the
 *     single cell that this netlist handles.
 *
 *   Add ports to the root with edif_portconfig
 *     The design may, if it is a macro to be included in a larger
 *     design, include ports. These are discovered by looking for port
 *     signals in the root module.
 *
 *   Declare external libraries with edif_xlibrary_create
 *     Normally, this is the single technology library that contains
 *     the primitive cells that the code generator intends to
 *     use. The library is given a name, such as VIRTEX or whatever
 *     the implementation tools expect. Cells are attached to the
 *     library later. An edif netlist may include multiple external
 *     references.
 *
 *   Declare primitives with edif_xcell_create and edif_cell_portconfig.
 *     These functions create CELL TYPES that are attached to an
 *     external library. The libraries are created by
 *     edif_xlibrary_create.
 *
 *     Cells can be created at any time before their first use. It
 *     therefore makes the most sense to not create the cell until it
 *     is certain that they are needed by the design.
 *
 *   Create instances and join them up
 *     The edif_cellref_t objects represent instances of cells, and
 *     are the devices of the generated netlist. These cellrefs are
 *     connected together by the use of edif_joint_t joints. The
 *     joints can be created from ivl_nexus_t objects, or from their
 *     own ether. This instantiating of cells and joining them
 *     together that is the most fun. It is the technology specific
 *     stuff that the code generator does.
 *
 *   Finally, print the result with edif_print(fd);
 *     This function writes the netlist in memory to an EDIF file on
 *     the stdio stream specified.
 *
 * This library is intended to be used once, to build up a netlist and
 * print it. All the names that are taken as const char* should be
 * somehow made permanent by the caller. Either they are constant
 * strings, or they are strduped as necessary to make them
 * permanent. The library will not duplicate them.
 */

/* TYPE DECLARATIONS */

/* This represents the entire EDIF design. You only need one of these
   to hold everything. */
typedef struct edif_s* edif_t;

/* Each external library of the design gets one of these. */
typedef struct edif_xlibrary_s* edif_xlibrary_t;

/* This represents a type of cell. */
typedef struct edif_cell_s* edif_cell_t;

/* A cellref is an *instance* of a cell. */
typedef struct edif_cellref_s* edif_cellref_t;

/* This represents a generic joint. Cell ports are connected by being
   associated with a joint. These can be bound to an ivl_nexus_t
   object, of stand along. */
typedef struct edif_joint_s* edif_joint_t;

/* This structure defines a table that can be attached to an xlibrary
   to incorporate black-box cells to the library. The cell_name is the
   name that may be passed to the edif_xlibrary_findcell function, and
   the function pointer points to a function that creates the cell and
   defines ports for it. A real celltable is terminated by an entry
   with a null pointer for the cell_name. */
struct edif_xlib_celltable {
      const char*cell_name;
      edif_cell_t (*cell_func)(edif_xlibrary_t xlib);
};

/* FUNCTIONS */


/* Start a new EDIF design. The design_name will be used as the name
   of the top-mode module of the design. */
extern edif_t edif_create(const char*design_name, unsigned nports);

/* macro ports to the design are handled by this library similar to
   cells. The user creates ports with this function. This function
   configures the sole "port" of the cell with the name and dir passed
   in. The direction, in this case, is the *interface* direction. */
extern void edif_portconfig(edif_t edf, unsigned idx,
			    const char*name, ivl_signal_port_t dir);

/* This is like edif_add_to_joint, but works with the edif port. */
extern void edif_port_to_joint(edif_joint_t jnt, edif_t edf, unsigned port);

/* The design may have properties attached to it. These properties
   will be attached to the instance declared in the footer of the EDIF
   file. */
extern void edif_pstring(edif_t edf, const char*name, const char*value);

/* Create an external library and attach it to the edif design. This
   will lead to a (external ...) declaration of cells that can be used
   by the design. */
extern edif_xlibrary_t edif_xlibrary_create(edif_t edf, const char*name);

extern void edif_xlibrary_set_celltable(edif_xlibrary_t lib,
				       const struct edif_xlib_celltable*table);


/* External libraries can be searched for existing cells, given a
   string name. This function searches for the cell by name, and
   returns it. */
extern edif_cell_t edif_xlibrary_findcell(edif_xlibrary_t lib,
					  const char*cell_name);

/* Similar to the above, but it gets the information it needs from the
   ivl_scope_t object. */
extern edif_cell_t edif_xlibrary_scope_cell(edif_xlibrary_t xlib,
					    ivl_scope_t scope);

/* Create a new cell, attached to the external library. Specify the
   number of ports that the cell has. The edif_cell_portconfig
   function is then used to assign name and direction to each of the
   ports.

   The cell has a number of pins that are referenced by their number
   from 0 to nports-1. You need to remember the pin numbers for the
   named ports for use when joining that pin to an edif_joint_t.

   Cellrefs get their port characteristics from the cell that they are
   created from. So the pinouts of cellrefs match the pinout of the
   associated cell. */
extern edif_cell_t edif_xcell_create(edif_xlibrary_t, const char*name,
				     unsigned nports);
extern void edif_cell_portconfig(edif_cell_t cell, unsigned idx,
				 const char*name, ivl_signal_port_t dir);

/* Attach a property to a cell port. */
extern void edif_cell_port_pstring(edif_cell_t cell, unsigned idx,
				   const char*name, const char*value);

/* Cells may have properties attached to them. These properties are
   included in the library declaration for the cell, instead of the
   cell instances. */
extern void edif_cell_pstring(edif_cell_t cell, const char*name,
			      const char*value);
extern void edif_cell_pinteger(edif_cell_t cell, const char*name,
			       int value);


/* Ports of cells are normally referenced by their port number. If you
   forget what that number is, this function can look it up by name. */
extern unsigned edif_cell_port_byname(edif_cell_t cell, const char*name);


/* Create and instance from a cell. The instance refers to the cell,
   which is a type, and contains pips for pins. */
extern edif_cellref_t edif_cellref_create(edif_t edf, edif_cell_t cell);

/* Instances can have properties attached to them. The name and value
   given here are turned into a (property <name> (string "val"))
   expression attached to the instance.

   Examples of string properties commonly attached to cellref devices
   include such things as the INIT=<value> to initialize LUT cells in
   FPGA devices. */
extern void edif_cellref_pstring(edif_cellref_t ref, const char*name,
				 const char*value);
extern void edif_cellref_pinteger(edif_cellref_t ref, const char*name,
				  int value);

/* This function gets the joint associated with a nexus. This will
   create a joint if necessary. */
extern edif_joint_t edif_joint_of_nexus(edif_t edf, ivl_nexus_t nex);

/* For linking cells outside the ivl netlist, this function creates an
   anonymous joint. */
extern edif_joint_t edif_joint_create(edif_t edf);

/* Renaming a joint causes it to take on a name when external tools
   view the EDIF file. */
extern void edif_joint_rename(edif_joint_t jnt, const char*name);

/* Given a joint, this function adds the cell reference. */
extern void edif_add_to_joint(edif_joint_t jnt,
			      edif_cellref_t cell,
			      unsigned port);

/*
 * Print the entire design. This should only be done after the design
 * is completely assembled.
 */
extern void edif_print(FILE*fd, edif_t design);

#endif /* IVL_edif_H */
