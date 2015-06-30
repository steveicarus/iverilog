/*
 * Copyright CERN 2015
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

#include "std_funcs.h"
#include "std_types.h"
#include "scope.h"

static std::map<perm_string,SubprogramHeader*> std_subprograms;

// Special case: to_integer function
static class SubprogramToInteger : public SubprogramHeader {
    public:
      SubprogramToInteger()
          : SubprogramHeader(perm_string::literal("to_integer"), NULL, &primitive_REAL) {
          ports_ = new std::list<InterfacePort*>();
          ports_->push_back(new InterfacePort(&primitive_INTEGER));
      }

      bool is_std() const { return true; }

      int emit_name(const std::vector<Expression*>&argv,
                    std::ostream&out, Entity*ent, ScopeBase*scope) const {
          bool signed_flag = false;

          // to_integer converts unsigned to natural
          //                     signed   to integer
          // try to determine the converted type
          const VType*type = argv[0]->probe_type(ent, scope);
          const VTypeArray*array = dynamic_cast<const VTypeArray*>(type);

          if(array) {
              signed_flag = array->signed_vector();
          } else {
              cerr << get_fileline() << ": sorry: Could not determine the "
                   << "expression sign. Output may be erroneous." << endl;
              return 1;
          }

          out << (signed_flag ? "$signed" : "$unsigned");
          return 0;
      }
}*fn_to_integer;

// Special case: size casting (e.g. conv_std_logic_vector() / resize()).
static class SubprogramSizeCast : public SubprogramHeader {
    public:
      SubprogramSizeCast(perm_string nam)
          : SubprogramHeader(nam, NULL, &primitive_STDLOGIC_VECTOR) {
          ports_ = new std::list<InterfacePort*>();
          ports_->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
          ports_->push_back(new InterfacePort(&primitive_INTEGER));
      }

      bool is_std() const { return true; }

      int emit_name(const std::vector<Expression*>&argv,
                    std::ostream&out, Entity*ent, ScopeBase*scope) const {
          int64_t use_size;
          bool rc = argv[1]->evaluate(ent, scope, use_size);

          if(!rc) {
              cerr << get_fileline() << ": sorry: Could not evaluate the "
                   << "expression size. Size casting impossible." << endl;
              return 1;
          }

          out << use_size << "'";
          return 0;
      }

      int emit_args(const std::vector<Expression*>&argv,
                    std::ostream&out, Entity*ent, ScopeBase*scope) const {

          return argv[0]->emit(out, ent, scope);
      }
}*fn_conv_std_logic_vector, *fn_resize;

static SubprogramBuiltin*fn_std_logic_vector;
static SubprogramBuiltin*fn_to_unsigned;
static SubprogramBuiltin*fn_unsigned;
static SubprogramBuiltin*fn_integer;

static SubprogramBuiltin*fn_rising_edge;
static SubprogramBuiltin*fn_falling_edge;

static SubprogramBuiltin*fn_and_reduce;
static SubprogramBuiltin*fn_or_reduce;

void preload_std_funcs(void)
{
    /* numeric_std library
     * function unsigned
     */
    std::list<InterfacePort*>*fn_unsigned_args = new std::list<InterfacePort*>();
    fn_unsigned_args->push_back(new InterfacePort(&primitive_INTEGER));
    fn_unsigned = new SubprogramBuiltin(perm_string::literal("unsigned"),
                                           perm_string::literal("$unsigned"),
                                           fn_unsigned_args, &primitive_UNSIGNED);
    std_subprograms[fn_unsigned->name()] = fn_unsigned;

    /* function integer
     */
    std::list<InterfacePort*>*fn_integer_args = new std::list<InterfacePort*>();
    fn_integer_args->push_back(new InterfacePort(&primitive_INTEGER));
    fn_integer = new SubprogramBuiltin(perm_string::literal("integer"),
                                           perm_string::literal("$signed"),
                                           fn_integer_args, &primitive_INTEGER);
    std_subprograms[fn_integer->name()] = fn_integer;

    /* function std_logic_vector
       Special case: The std_logic_vector function casts its
       argument to std_logic_vector. Internally, we don't
       have to do anything for that to work.
    */
    std::list<InterfacePort*>*fn_std_logic_vector_args = new std::list<InterfacePort*>();
    fn_std_logic_vector_args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
    fn_std_logic_vector = new SubprogramBuiltin(perm_string::literal("std_logic_vector"),
                                           empty_perm_string,
                                           fn_std_logic_vector_args, &primitive_STDLOGIC_VECTOR);
    std_subprograms[fn_std_logic_vector->name()] = fn_std_logic_vector;

    /* function resize
     */
    fn_resize = new SubprogramSizeCast(perm_string::literal("resize"));
    std_subprograms[fn_resize->name()] = fn_resize;

    /* function conv_std_logic_vector
     */
    fn_conv_std_logic_vector = new SubprogramSizeCast(perm_string::literal("conv_std_logic_vector"));
    std_subprograms[fn_conv_std_logic_vector->name()] = fn_conv_std_logic_vector;

    /* numeric_bit library
     * function to_integer (arg: unsigned) return natural;
     * function to_integer (arg: signed) return integer;
     */
    fn_to_integer = new SubprogramToInteger();
    std_subprograms[fn_to_integer->name()] = fn_to_integer;

    /* std_logic_1164 library
     * function rising_edge  (signal s : std_ulogic) return boolean;
     */
    std::list<InterfacePort*>*fn_rising_edge_args = new std::list<InterfacePort*>();
    fn_rising_edge_args->push_back(new InterfacePort(&primitive_STDLOGIC));
    fn_rising_edge = new SubprogramBuiltin(perm_string::literal("rising_edge"),
                                           perm_string::literal("$ivlh_rising_edge"),
                                           fn_rising_edge_args, &type_BOOLEAN);
    std_subprograms[fn_rising_edge->name()] = fn_rising_edge;

    /* std_logic_1164 library
     * function falling_edge (signal s : std_ulogic) return boolean;
     */
    std::list<InterfacePort*>*fn_falling_edge_args = new std::list<InterfacePort*>();
    fn_falling_edge_args->push_back(new InterfacePort(&primitive_STDLOGIC));
    fn_falling_edge = new SubprogramBuiltin(perm_string::literal("falling_edge"),
                                           perm_string::literal("$ivlh_falling_edge"),
                                           fn_falling_edge_args, &type_BOOLEAN);
    std_subprograms[fn_falling_edge->name()] = fn_falling_edge;

    /* reduce_pack library
     * function or_reduce(arg : std_logic_vector) return std_logic;
     */
    std::list<InterfacePort*>*fn_or_reduce_args = new std::list<InterfacePort*>();
    fn_or_reduce_args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
    fn_or_reduce = new SubprogramBuiltin(perm_string::literal("or_reduce"),
                                           perm_string::literal("|"),
                                           fn_or_reduce_args, &primitive_STDLOGIC);
    std_subprograms[fn_or_reduce->name()] = fn_or_reduce;

    /* reduce_pack library
     * function and_reduce(arg : std_logic_vector) return std_logic;
     */
    std::list<InterfacePort*>*fn_and_reduce_args = new std::list<InterfacePort*>();
    fn_and_reduce_args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
    fn_and_reduce = new SubprogramBuiltin(perm_string::literal("and_reduce"),
                                           perm_string::literal("&"),
                                           fn_and_reduce_args, &primitive_STDLOGIC);
    std_subprograms[fn_and_reduce->name()] = fn_and_reduce;

    /* fixed_pkg library
     * function to_unsigned (
     *   arg           : ufixed;             -- fixed point input
     *   constant size : natural)            -- length of output
     * return unsigned;
     */
    std::list<InterfacePort*>*fn_to_unsigned_args = new std::list<InterfacePort*>();
    fn_to_unsigned_args->push_back(new InterfacePort(&primitive_REAL));
    fn_to_unsigned_args->push_back(new InterfacePort(&primitive_NATURAL));
    fn_to_unsigned = new SubprogramBuiltin(perm_string::literal("to_unsigned"),
                                           perm_string::literal("$ivlh_to_unsigned"),
                                           fn_to_unsigned_args, &primitive_UNSIGNED);
    std_subprograms[fn_to_unsigned->name()] = fn_to_unsigned;
}

void delete_std_funcs()
{
    for(std::map<perm_string,SubprogramHeader*>::iterator it = std_subprograms.begin();
            it != std_subprograms.end(); ++it) {
        delete it->second;
    }
}

SubprogramHeader*find_std_subprogram(perm_string name)
{
      map<perm_string,SubprogramHeader*>::const_iterator cur = std_subprograms.find(name);
      if (cur != std_subprograms.end())
          return cur->second;

      return NULL;
}
