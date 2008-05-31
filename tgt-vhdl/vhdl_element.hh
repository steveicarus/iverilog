/*
 *  VHDL abstract syntax elements.
 *
 *  Copyright (C) 2008  Nick Gasson (nick@nickg.me.uk)
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

#ifndef INC_VHDL_ELEMENT_HH
#define INC_VHDL_ELEMENT_HH

#include <fstream>
#include <list>
#include <string>


class vhdl_element;
typedef std::list<vhdl_element*> element_list_t; 

class vhdl_element {
public:
   virtual ~vhdl_element() {}
   
   virtual void emit(std::ofstream &of, int level=0) const = 0;

   void set_comment(std::string comment);
protected:
   void emit_comment(std::ofstream &of, int level) const;
private:
   std::string comment_;
};

class vhdl_conc_stmt : public vhdl_element {
public:
   virtual ~vhdl_conc_stmt() {}
};

typedef std::list<vhdl_conc_stmt*> conc_stmt_list_t;

class vhdl_seq_stmt : public vhdl_element {
public:
   virtual ~vhdl_seq_stmt() {}
};

typedef std::list<vhdl_seq_stmt*> seq_stmt_list_t;

class vhdl_process : public vhdl_conc_stmt {
public:
   vhdl_process(const char *name = NULL);
   virtual ~vhdl_process();

   void emit(std::ofstream &of, int level) const;
   void add_stmt(vhdl_seq_stmt* stmt);
private:
   seq_stmt_list_t stmts_;
   const char *name_;
};

class vhdl_arch : public vhdl_element {
public:
   vhdl_arch(const char *entity, const char *name="Behavioural");
   virtual ~vhdl_arch();

   void emit(std::ofstream &of, int level=0) const;
   void add_stmt(vhdl_conc_stmt* stmt);
private:
   conc_stmt_list_t stmts_;
   const char *name_, *entity_;
};

class vhdl_entity : public vhdl_element {
public:
   vhdl_entity(const char *name, vhdl_arch *arch);
   virtual ~vhdl_entity();

   void emit(std::ofstream &of, int level=0) const;
   vhdl_arch *get_arch() const { return arch_; }
   const char *get_name() const { return name_; }
private:
   const char *name_;
   vhdl_arch *arch_;  // Entity may only have a single architecture
};

typedef std::list<vhdl_entity*> entity_list_t;


#endif

