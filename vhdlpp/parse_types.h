#ifndef IVL_parse_types_H
#define IVL_parse_types_H
/*
 * Copyright (c) 2011,2014 Stephen Williams (steve@icarus.com)
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
      named_expr_t (perm_string n, Expression*e) : name_(n), expr_(e) { }

      perm_string name() const { return name_; }
      Expression* expr() const { return expr_; }
      void dump(ostream&out, int indent) const;
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
      typedef enum { ALL = 0, OTHERS, NONE } application_domain_t;

      instant_list_t(application_domain_t d, std::list<perm_string>* l) : domain_(d), labels_(l) {}
      ~instant_list_t() { delete labels_; }

      std::list<perm_string>* labels() const { return labels_; }
      application_domain_t domain() const { return domain_; }

      application_domain_t domain_;
      std::list<perm_string>* labels_;
};

class prange_t {
    public:
      prange_t(Expression* left, Expression* right, bool dir)
        : left_(left), right_(right), direction_(dir), auto_dir_(false) {}
      prange_t(const prange_t&other) :
          left_(other.left_->clone()), right_(other.right_->clone()),
          direction_(other.direction_), auto_dir_(other.auto_dir_) {}
      ~prange_t() { delete left_; delete right_; }
      void dump(ostream&out, int indent) const;

      inline Expression*msb() { return direction_? left_ : right_; }
      inline Expression*lsb() { return direction_? right_: left_;  }

      inline bool is_downto() const { return direction_; }
      inline void set_auto_dir(bool enabled = true) { auto_dir_ = enabled; };
      inline bool is_auto_dir() const { return auto_dir_; }

      inline Expression*expr_left() { return left_; }
      inline Expression*expr_right() { return right_; }

    private:
      Expression *left_, *right_;
      bool direction_;
      bool auto_dir_;

    private: //not implemented
      prange_t operator=(const prange_t&);
};

struct adding_term {
      ExpArithmetic::fun_t op;
      Expression*term;
};

#endif /* IVL_parse_types_H */
