/*
 * Copyright CERN 2016-2021
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

using namespace std;

static std::map<perm_string,SubHeaderList> std_subprograms;

void register_std_subprogram(SubprogramHeader*header)
{
    std_subprograms[header->name()].push_back(header);
}

// Special case: to_integer function
class SubprogramToInteger : public SubprogramStdHeader {
    public:
      SubprogramToInteger()
          : SubprogramStdHeader(perm_string::literal("to_integer"), NULL, &primitive_REAL) {
          ports_ = new list<InterfacePort*>();
          ports_->push_back(new InterfacePort(&primitive_INTEGER));
      }

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
};

// Special case: size casting (e.g. conv_std_logic_vector() / resize()).
class SubprogramSizeCast : public SubprogramStdHeader {
    public:
      explicit SubprogramSizeCast(perm_string nam, const VType*base, const VType*target)
          : SubprogramStdHeader(nam, NULL, target) {
          ports_ = new list<InterfacePort*>();
          ports_->push_back(new InterfacePort(base));
          ports_->push_back(new InterfacePort(&primitive_NATURAL));
      }

      int emit_name(const std::vector<Expression*>&,
                    std::ostream&, Entity*, ScopeBase*) const {
          return 0;
      }

      int emit_args(const std::vector<Expression*>&argv,
                    std::ostream&out, Entity*ent, ScopeBase*scope) const {
          int64_t new_size, old_size;

          const VType*type = argv[0]->probe_type(ent, scope);

          if(!type) {
              cerr << get_fileline() << ": sorry: Could not determine "
                   << "the argument type. Size casting impossible." << endl;
              return 1;
          }

          old_size = type->get_width(scope);

          if(old_size <= 0) {
              cerr << get_fileline() << ": sorry: Could not determine "
                   << "the argument size. Size casting impossible." << endl;
              return 1;
          }

          if(!argv[1]->evaluate(ent, scope, new_size)) {
              cerr << get_fileline() << ": sorry: Could not evaluate the requested"
                   << "expression size. Size casting impossible." << endl;
              return 1;
          }


          out << new_size << "'(" << old_size << "'(";

          if(const VTypeArray*arr = dynamic_cast<const VTypeArray*>(type))
                out << (arr->signed_vector() ? "$signed" : "$unsigned");

          out << "(";
          bool res = argv[0]->emit(out, ent, scope);
          out << ")))";

          return res;
      }
};

class SubprogramReadWrite : public SubprogramBuiltin {
    public:
      SubprogramReadWrite(perm_string nam, perm_string newnam, bool hex = false)
          : SubprogramBuiltin(nam, newnam, NULL, NULL), hex_format_(hex) {
            ports_ = new list<InterfacePort*>();
            ports_->push_back(new InterfacePort(&primitive_STRING));
            ports_->push_back(new InterfacePort(NULL));
          }

      // Format types handled by $ivlh_read/write (see vpi/vhdl_textio.c)
      enum format_t { FORMAT_STD, FORMAT_BOOL, FORMAT_TIME, FORMAT_HEX, FORMAT_STRING };

      int emit_args(const std::vector<Expression*>&argv,
                    std::ostream&out, Entity*ent, ScopeBase*scope) const {

          int errors = 0;

          for(int i = 0; i < 2; ++i) {
            errors += argv[i]->emit(out, ent, scope);
            out << ", ";
          }

          const VType*arg_type = argv[1]->probe_type(ent, scope);

          while(const VTypeDef*tdef = dynamic_cast<const VTypeDef*>(arg_type))
            arg_type = tdef->peek_definition();

          // Pick the right format
          if(hex_format_) {
              out << FORMAT_HEX;
          } else if(arg_type) {
              if(arg_type->type_match(&primitive_TIME)) {
                  out << FORMAT_TIME;
              } else if(arg_type->type_match(&type_BOOLEAN)) {
                  out << FORMAT_BOOL;
              } else if(arg_type->type_match(&primitive_CHARACTER)) {
                  out << FORMAT_STRING;
              } else {
                  const VTypeArray*arr = dynamic_cast<const VTypeArray*>(arg_type);

                  if(arr && arr->element_type() == &primitive_CHARACTER)
                    out << FORMAT_STRING;
                  else
                    out << FORMAT_STD;
              }
          } else {
              out << FORMAT_STD;
          }

          return errors;
      }

    private:
      bool hex_format_;
};

void preload_std_funcs(void)
{
    list<InterfacePort*>*args;

    /* function now */
    SubprogramBuiltin*fn_now = new SubprogramBuiltin(perm_string::literal("now"),
                                            perm_string::literal("$time"), NULL, NULL);
    register_std_subprogram(fn_now);

    /* numeric_std library
     * function unsigned
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_INTEGER));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("unsigned"),
                                           perm_string::literal("$unsigned"),
                                           args, &primitive_UNSIGNED));

    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("unsigned"),
                                           perm_string::literal("$unsigned"),
                                           args, &primitive_UNSIGNED));

    /* function integer
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_REAL));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("integer"),
                                           perm_string::literal("int'"),
                                           args, &primitive_INTEGER));

    /* function std_logic_vector
       Special case: The std_logic_vector function casts its
       argument to std_logic_vector. Internally, we don't
       have to do anything for that to work.
    */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_SIGNED));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("std_logic_vector"),
                                           empty_perm_string,
                                           args, &primitive_STDLOGIC_VECTOR));

    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_UNSIGNED));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("std_logic_vector"),
                                           empty_perm_string,
                                           args, &primitive_STDLOGIC_VECTOR));

    /* numeric_std library
     * function shift_left (arg: unsigned; count: natural) return unsigned;
     * function shift_left (arg: signed; count: natural) return signed;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_UNSIGNED));
    args->push_back(new InterfacePort(&primitive_NATURAL));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("shift_left"),
                                           perm_string::literal("$ivlh_shift_left"),
                                           args, &primitive_UNSIGNED));

    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_SIGNED));
    args->push_back(new InterfacePort(&primitive_NATURAL));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("shift_left"),
                                           perm_string::literal("$ivlh_shift_left"),
                                           args, &primitive_SIGNED));

    /* numeric_std library
     * function shift_right (arg: unsigned; count: natural) return unsigned;
     * function shift_right (arg: signed; count: natural) return signed;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_UNSIGNED));
    args->push_back(new InterfacePort(&primitive_NATURAL));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("shift_right"),
                                           perm_string::literal("$ivlh_shift_right"),
                                           args, &primitive_UNSIGNED));

    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_SIGNED));
    args->push_back(new InterfacePort(&primitive_NATURAL));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("shift_right"),
                                           perm_string::literal("$ivlh_shift_right"),
                                           args, &primitive_SIGNED));

    /* function resize
     */
    register_std_subprogram(new SubprogramSizeCast(perm_string::literal("resize"),
                &primitive_UNSIGNED, &primitive_UNSIGNED));

    register_std_subprogram(new SubprogramSizeCast(perm_string::literal("resize"),
                &primitive_SIGNED, &primitive_SIGNED));

    /* std_logic_arith library
     * function conv_std_logic_vector(arg: integer; size: integer) return std_logic_vector;
     */
    register_std_subprogram(new SubprogramSizeCast(
                perm_string::literal("conv_std_logic_vector"),
                &primitive_INTEGER, &primitive_STDLOGIC_VECTOR));

    /* numeric_bit library
     * function to_integer (arg: unsigned) return natural;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_UNSIGNED));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("to_integer"),
                perm_string::literal("$unsigned"),
                args, &primitive_NATURAL));

    /* numeric_bit library
     * function to_integer (arg: signed) return integer;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_SIGNED));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("to_integer"),
                perm_string::literal("$signed"),
                args, &primitive_INTEGER));

    /* std_logic_1164 library
     * function to_bit (signal s : std_ulogic) return bit;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_STDLOGIC));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("to_bit"),
                                           empty_perm_string,
                                           args, &primitive_BIT));

    /* std_logic_1164 library
     * function to_bitvector (signal s : std_logic_vector) return bit_vector;
     * function to_bitvector (signal s : std_ulogic_vector) return bit_vector;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("to_bitvector"),
                                           empty_perm_string,
                                           args, &primitive_BIT_VECTOR));

    /* std_logic_1164 library
     * function rising_edge  (signal s : std_ulogic) return boolean;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_STDLOGIC));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("rising_edge"),
                                           perm_string::literal("$ivlh_rising_edge"),
                                           args, &type_BOOLEAN));

    /* std_logic_1164 library
     * function falling_edge (signal s : std_ulogic) return boolean;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_STDLOGIC));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("falling_edge"),
                                           perm_string::literal("$ivlh_falling_edge"),
                                           args, &type_BOOLEAN));

    /* reduce_pack library
     * function or_reduce(arg : std_logic_vector) return std_logic;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("or_reduce"),
                                           perm_string::literal("|"),
                                           args, &primitive_STDLOGIC));

    /* reduce_pack library
     * function and_reduce(arg : std_logic_vector) return std_logic;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("and_reduce"),
                                           perm_string::literal("&"),
                                           args, &primitive_STDLOGIC));

    /* fixed_pkg library
     * function to_unsigned (
     *   arg           : ufixed;             -- fixed point input
     *   constant size : natural)            -- length of output
     * return unsigned;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_REAL));
    args->push_back(new InterfacePort(&primitive_NATURAL));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("to_unsigned"),
                                           perm_string::literal("$ivlh_to_unsigned"),
                                           args, &primitive_UNSIGNED));
    /* numeric_std library
     * function to_unsigned(arg, size : natural) return unsigned;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_NATURAL));
    args->push_back(new InterfacePort(&primitive_NATURAL));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("to_unsigned"),
                                           perm_string::literal("$ivlh_to_unsigned"),
                                           args, &primitive_UNSIGNED));

    /* numeric_std library
     * function to_unsigned(arg : std_logic_vector, size : natural) return unsigned;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
    args->push_back(new InterfacePort(&primitive_NATURAL));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("to_unsigned"),
                                           perm_string::literal("$ivlh_to_unsigned"),
                                           args, &primitive_UNSIGNED));

    /* procedure file_open (file f: text; filename: in string, file_open_kind: in mode);
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
    args->push_back(new InterfacePort(&primitive_STRING, PORT_IN));
    args->push_back(new InterfacePort(&type_FILE_OPEN_KIND, PORT_IN));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("file_open"),
                                          perm_string::literal("$ivlh_file_open"),
                                          args, NULL));

    /* procedure file_open (status: out file_open_status, file f: text; filename: in string, file_open_kind: in mode);
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&type_FILE_OPEN_STATUS, PORT_OUT));
    args->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
    args->push_back(new InterfacePort(&primitive_STRING, PORT_IN));
    args->push_back(new InterfacePort(&type_FILE_OPEN_KIND, PORT_IN));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("file_open"),
                                          perm_string::literal("$ivlh_file_open"),
                                          args, NULL));

    /* std.textio library
     * procedure file_close (file f: text);
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("file_close"),
                                          perm_string::literal("$fclose"),
                                          args, NULL));

    /* std.textio library
     * procedure read (l: inout line; value: out bit/bit_vector/boolean/character/integer/real/string/time);
     */
    register_std_subprogram(new SubprogramReadWrite(perm_string::literal("read"),
                                      perm_string::literal("$ivlh_read")));

    /* std.textio library
     * procedure write (l: inout line; value: out bit/bit_vector/boolean/character/integer/real/string/time);
     */
    register_std_subprogram(new SubprogramReadWrite(perm_string::literal("write"),
                                      perm_string::literal("$ivlh_write")));

    /* std.textio library
     * procedure hread (l: inout line; value: out bit/bit_vector/boolean/character/integer/real/string/time);
     */
    register_std_subprogram(new SubprogramReadWrite(perm_string::literal("hread"),
                                          perm_string::literal("$ivlh_read"), true));

    /* std.textio library
     * procedure hwrite (l: inout line; value: out bit/bit_vector/boolean/character/integer/real/string/time);
     */
    register_std_subprogram(new SubprogramReadWrite(perm_string::literal("hwrite"),
                                           perm_string::literal("$ivlh_write"), true));

    /* std.textio library
     * procedure readline (file f: text; l: inout line);
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
    args->push_back(new InterfacePort(&primitive_STRING, PORT_OUT));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("readline"),
                                       perm_string::literal("$ivlh_readline"),
                                       args, NULL));

    /* std.textio library
     * procedure writeline (file f: text; l: inout line);
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
    args->push_back(new InterfacePort(&primitive_STRING, PORT_IN));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("writeline"),
                                       perm_string::literal("$ivlh_writeline"),
                                       args, NULL));

    /* function endline (file f: text) return boolean;
     */
    args = new list<InterfacePort*>();
    args->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
    register_std_subprogram(new SubprogramBuiltin(perm_string::literal("endfile"),
                                       perm_string::literal("$feof"),
                                       args, &type_BOOLEAN));
}

void delete_std_funcs()
{
    for(std::map<perm_string,SubHeaderList>::iterator cur = std_subprograms.begin();
            cur != std_subprograms.end(); ++cur) {
	    for(SubHeaderList::const_iterator it = cur->second.begin();
			it != cur->second.end(); ++it) {
                delete *it;
            }
    }
}

SubHeaderList find_std_subprogram(perm_string name)
{
    map<perm_string,SubHeaderList>::const_iterator cur = std_subprograms.find(name);
    if(cur != std_subprograms.end())
        return cur->second;

    return SubHeaderList();
}
