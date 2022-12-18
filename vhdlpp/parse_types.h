#ifndef IVL_parse_types_H
#define IVL_parse_types_H
/*
 * Copyright (c) 2011-2021 Stephen Williams (steve@icarus.com)
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

# include  "StringHeap.h"
# include  "expression.h"

class named_expr_t {

    public:
      named_expr_t(perm_string n, Expression*e) : name_(n), expr_(e) { }
      ~named_expr_t() { delete expr_; }

      perm_string name() const { return name_; }
      Expression* expr() const { return expr_; }
      void dump(std::ostream&out, int indent) const;
    private:
      perm_string name_;
      Expression* expr_;

    private: // Not implemented
      named_expr_t(const named_expr_t&);
      named_expr_t& operator = (const named_expr_t&);
};

class entity_aspect_t {
    public:
      typedef enum { ENTITY = 0, CONFIGURATION, OPEN } entity_aspect_type_t;

      entity_aspect_t(entity_aspect_type_t t, ExpName* n) : type_(t), name_(n) {}
      ~entity_aspect_t() { delete name_; }

      ExpName* name() const { return name_; }
      entity_aspect_type_t type() const {return type_; }

      entity_aspect_type_t type_;
      ExpName* name_;
};

class instant_list_t {
    public:
      typedef enum { ALL = 0, OTHERS, NO_DOMAIN } application_domain_t;

      instant_list_t(application_domain_t d, std::list<perm_string>* l) : domain_(d), labels_(l) {}
      ~instant_list_t() { delete labels_; }

      std::list<perm_string>* labels() const { return labels_; }
      application_domain_t domain() const { return domain_; }

      application_domain_t domain_;
      std::list<perm_string>* labels_;
};

struct adding_term {
      ExpArithmetic::fun_t op;
      Expression*term;
};

// Stores information for file declarations containing a file name and open mode
// (VHDL-2008 6.4.2.5)
class file_open_info_t {
    public:
      explicit file_open_info_t(ExpString*filename__, ExpName*kind__ = NULL)
        : kind_(kind__), filename_(filename__) {
          // By default files are opened in read-only mode
          if(!kind_) kind_ = new ExpName(perm_string::literal("read_mode"));
      }
      ~file_open_info_t() { delete kind_; delete filename_; }

      ExpName*kind() { return kind_; }
      ExpString*filename() { return filename_; }

    private:
      ExpName*kind_;
      ExpString*filename_;
};

#endif /* IVL_parse_types_H */
