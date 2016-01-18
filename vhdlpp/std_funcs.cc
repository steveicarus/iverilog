/*
 * Copyright CERN 2016
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

static inline void register_std_subprogram(SubprogramHeader*header)
{
    std_subprograms[header->name()] = header;
}

// Special case: to_integer function
static class SubprogramToInteger : public SubprogramStdHeader {
    public:
      SubprogramToInteger()
          : SubprogramStdHeader(perm_string::literal("to_integer"), NULL, &primitive_REAL) {
          ports_ = new std::list<InterfacePort*>();
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
}*fn_to_integer;

// Special case: size casting (e.g. conv_std_logic_vector() / resize()).
static class SubprogramSizeCast : public SubprogramStdHeader {
    public:
      explicit SubprogramSizeCast(perm_string nam)
          : SubprogramStdHeader(nam, NULL, &primitive_STDLOGIC_VECTOR) {
          ports_ = new std::list<InterfacePort*>();
          ports_->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
          ports_->push_back(new InterfacePort(&primitive_INTEGER));
      }

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

static class SubprogramReadWrite : public SubprogramBuiltin {
    public:
      SubprogramReadWrite(perm_string nam, perm_string newnam)
          : SubprogramBuiltin(nam, newnam, NULL, NULL) {
            ports_ = new std::list<InterfacePort*>();
            ports_->push_back(new InterfacePort(&primitive_STRING, PORT_INOUT));
            ports_->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR, PORT_INOUT));
            ports_->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
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
          const VTypeArray*arr = dynamic_cast<const VTypeArray*>(arg_type);
          const VTypePrimitive*prim = dynamic_cast<const VTypePrimitive*>(arg_type);

          // Pick the right format
          if(prim && prim->type() == VTypePrimitive::TIME)
              out << FORMAT_TIME;
          else if(arg_type && arg_type->type_match(&type_BOOLEAN))
              out << FORMAT_BOOL;
          else if((arg_type && arg_type->type_match(&primitive_CHARACTER)) ||
                 (arr && arr->element_type() == &primitive_CHARACTER))
              out << FORMAT_STRING;
          else
              out << FORMAT_STD;

          return errors;
      }
}*fn_read, *fn_write;

static class SubprogramHexReadWrite : public SubprogramBuiltin {
    public:
      SubprogramHexReadWrite(perm_string nam, perm_string newnam)
          : SubprogramBuiltin(nam, newnam, NULL, NULL) {
            ports_ = new std::list<InterfacePort*>();
            ports_->push_back(new InterfacePort(&primitive_STRING, PORT_INOUT));
            ports_->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR, PORT_INOUT));
            ports_->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
          }

      int emit_args(const std::vector<Expression*>&argv,
                    std::ostream&out, Entity*ent, ScopeBase*scope) const {

          int errors = 0;

          for(int i = 0; i < 2; ++i) {
            errors += argv[i]->emit(out, ent, scope);
            out << ", ";
          }

          out << SubprogramReadWrite::FORMAT_HEX;

          return errors;
      }
}*fn_hread, *fn_hwrite;

void preload_std_funcs(void)
{
    /* function now */
    SubprogramBuiltin*fn_now = new SubprogramBuiltin(perm_string::literal("now"),
                                            perm_string::literal("$time"), NULL, NULL);
    register_std_subprogram(fn_now);

    /* numeric_std library
     * function unsigned
     */
    std::list<InterfacePort*>*fn_unsigned_args = new std::list<InterfacePort*>();
    fn_unsigned_args->push_back(new InterfacePort(&primitive_INTEGER));
    SubprogramBuiltin*fn_unsigned = new SubprogramBuiltin(perm_string::literal("unsigned"),
                                           perm_string::literal("$unsigned"),
                                           fn_unsigned_args, &primitive_UNSIGNED);
    register_std_subprogram(fn_unsigned);

    /* function integer
     */
    std::list<InterfacePort*>*fn_integer_args = new std::list<InterfacePort*>();
    fn_integer_args->push_back(new InterfacePort(&primitive_INTEGER));
    SubprogramBuiltin*fn_integer = new SubprogramBuiltin(perm_string::literal("integer"),
                                           perm_string::literal("$signed"),
                                           fn_integer_args, &primitive_INTEGER);
    register_std_subprogram(fn_integer);

    /* function std_logic_vector
       Special case: The std_logic_vector function casts its
       argument to std_logic_vector. Internally, we don't
       have to do anything for that to work.
    */
    std::list<InterfacePort*>*fn_std_logic_vector_args = new std::list<InterfacePort*>();
    fn_std_logic_vector_args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
    SubprogramBuiltin*fn_std_logic_vector = new SubprogramBuiltin(perm_string::literal("std_logic_vector"),
                                           empty_perm_string,
                                           fn_std_logic_vector_args, &primitive_STDLOGIC_VECTOR);
    register_std_subprogram(fn_std_logic_vector);

    /* numeric_std library
     * function shift_left (arg: unsigned; count: natural) return unsigned;
     */
    std::list<InterfacePort*>*fn_shift_left_args = new std::list<InterfacePort*>();
    fn_shift_left_args->push_back(new InterfacePort(&primitive_UNSIGNED));
    fn_shift_left_args->push_back(new InterfacePort(&primitive_UNSIGNED));
    SubprogramBuiltin*fn_shift_left = new SubprogramBuiltin(perm_string::literal("shift_left"),
                                           perm_string::literal("$ivlh_shift_left"),
                                           fn_shift_left_args, &primitive_UNSIGNED);
    register_std_subprogram(fn_shift_left);

    /* numeric_std library
     * function shift_right (arg: unsigned; count: natural) return unsigned;
     */
    std::list<InterfacePort*>*fn_shift_right_args = new std::list<InterfacePort*>();
    fn_shift_right_args->push_back(new InterfacePort(&primitive_UNSIGNED));
    fn_shift_right_args->push_back(new InterfacePort(&primitive_UNSIGNED));
    SubprogramBuiltin*fn_shift_right = new SubprogramBuiltin(perm_string::literal("shift_right"),
                                           perm_string::literal("$ivlh_shift_right"),
                                           fn_shift_right_args, &primitive_UNSIGNED);
    register_std_subprogram(fn_shift_right);

    /* function resize
     */
    fn_resize = new SubprogramSizeCast(perm_string::literal("resize"));
    register_std_subprogram(fn_resize);

    /* std_logic_arith library
     * function conv_std_logic_vector(arg: integer; size: integer) return std_logic_vector;
     */
    fn_conv_std_logic_vector = new SubprogramSizeCast(perm_string::literal("conv_std_logic_vector"));
    register_std_subprogram(fn_conv_std_logic_vector);

    /* numeric_bit library
     * function to_integer (arg: unsigned) return natural;
     * function to_integer (arg: signed) return integer;
     */
    fn_to_integer = new SubprogramToInteger();
    register_std_subprogram(fn_to_integer);

    /* std_logic_1164 library
     * function rising_edge  (signal s : std_ulogic) return boolean;
     */
    std::list<InterfacePort*>*fn_rising_edge_args = new std::list<InterfacePort*>();
    fn_rising_edge_args->push_back(new InterfacePort(&primitive_STDLOGIC));
    SubprogramBuiltin*fn_rising_edge = new SubprogramBuiltin(perm_string::literal("rising_edge"),
                                           perm_string::literal("$ivlh_rising_edge"),
                                           fn_rising_edge_args, &type_BOOLEAN);
    register_std_subprogram(fn_rising_edge);

    /* std_logic_1164 library
     * function falling_edge (signal s : std_ulogic) return boolean;
     */
    std::list<InterfacePort*>*fn_falling_edge_args = new std::list<InterfacePort*>();
    fn_falling_edge_args->push_back(new InterfacePort(&primitive_STDLOGIC));
    SubprogramBuiltin*fn_falling_edge = new SubprogramBuiltin(perm_string::literal("falling_edge"),
                                           perm_string::literal("$ivlh_falling_edge"),
                                           fn_falling_edge_args, &type_BOOLEAN);
    register_std_subprogram(fn_falling_edge);

    /* reduce_pack library
     * function or_reduce(arg : std_logic_vector) return std_logic;
     */
    std::list<InterfacePort*>*fn_or_reduce_args = new std::list<InterfacePort*>();
    fn_or_reduce_args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
    SubprogramBuiltin*fn_or_reduce = new SubprogramBuiltin(perm_string::literal("or_reduce"),
                                           perm_string::literal("|"),
                                           fn_or_reduce_args, &primitive_STDLOGIC);
    register_std_subprogram(fn_or_reduce);

    /* reduce_pack library
     * function and_reduce(arg : std_logic_vector) return std_logic;
     */
    std::list<InterfacePort*>*fn_and_reduce_args = new std::list<InterfacePort*>();
    fn_and_reduce_args->push_back(new InterfacePort(&primitive_STDLOGIC_VECTOR));
    SubprogramBuiltin*fn_and_reduce = new SubprogramBuiltin(perm_string::literal("and_reduce"),
                                           perm_string::literal("&"),
                                           fn_and_reduce_args, &primitive_STDLOGIC);
    register_std_subprogram(fn_and_reduce);

    /* fixed_pkg library
     * function to_unsigned (
     *   arg           : ufixed;             -- fixed point input
     *   constant size : natural)            -- length of output
     * return unsigned;
     */
    std::list<InterfacePort*>*fn_to_unsigned_args = new std::list<InterfacePort*>();
    fn_to_unsigned_args->push_back(new InterfacePort(&primitive_REAL));
    fn_to_unsigned_args->push_back(new InterfacePort(&primitive_NATURAL));
    SubprogramBuiltin*fn_to_unsigned = new SubprogramBuiltin(perm_string::literal("to_unsigned"),
                                           perm_string::literal("$ivlh_to_unsigned"),
                                           fn_to_unsigned_args, &primitive_UNSIGNED);
    register_std_subprogram(fn_to_unsigned);

    /* procedure file_open (file f: text; filename: in string, file_open_kind: in mode);
     */
    std::list<InterfacePort*>*fn_file_open_args = new std::list<InterfacePort*>();
    fn_file_open_args->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
    fn_file_open_args->push_back(new InterfacePort(&primitive_STRING, PORT_IN));
    fn_file_open_args->push_back(new InterfacePort(&type_FILE_OPEN_KIND, PORT_IN));
    SubprogramBuiltin*fn_file_open = new SubprogramBuiltin(perm_string::literal("file_open"),
                                          perm_string::literal("$ivlh_file_open"),
                                          fn_file_open_args, NULL);
    register_std_subprogram(fn_file_open);

    /* std.textio library
     * procedure file_close (file f: text);
     */
    std::list<InterfacePort*>*fn_file_close_args = new std::list<InterfacePort*>();
    fn_file_close_args->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
    SubprogramBuiltin*fn_file_close = new SubprogramBuiltin(perm_string::literal("file_close"),
                                          perm_string::literal("$fclose"),
                                          fn_file_close_args, NULL);
    register_std_subprogram(fn_file_close);

    /* std.textio library
     * procedure read (l: inout line; value: out bit/bit_vector/boolean/character/integer/real/string/time);
     */
    fn_read = new SubprogramReadWrite(perm_string::literal("read"),
                                      perm_string::literal("$ivlh_read"));
    register_std_subprogram(fn_read);

    /* std.textio library
     * procedure write (l: inout line; value: out bit/bit_vector/boolean/character/integer/real/string/time);
     */
    fn_write = new SubprogramReadWrite(perm_string::literal("write"),
                                      perm_string::literal("$ivlh_write"));
    register_std_subprogram(fn_write);

    /* std.textio library
     * procedure hread (l: inout line; value: out bit/bit_vector/boolean/character/integer/real/string/time);
     */
    fn_hread = new SubprogramHexReadWrite(perm_string::literal("hread"),
                                          perm_string::literal("$ivlh_read"));
    register_std_subprogram(fn_hread);

    /* std.textio library
     * procedure hwrite (l: inout line; value: out bit/bit_vector/boolean/character/integer/real/string/time);
     */
    fn_hwrite = new SubprogramHexReadWrite(perm_string::literal("hwrite"),
                                           perm_string::literal("$ivlh_write"));
    register_std_subprogram(fn_hwrite);

    /* std.textio library
     * procedure readline (file f: text; l: inout line);
     */
    std::list<InterfacePort*>*fn_readline_args = new std::list<InterfacePort*>();
    fn_readline_args->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
    fn_readline_args->push_back(new InterfacePort(&primitive_STRING, PORT_OUT));
    SubprogramBuiltin*fn_readline = new SubprogramBuiltin(perm_string::literal("readline"),
                                       perm_string::literal("$ivlh_readline"),
                                       fn_readline_args, NULL);
    register_std_subprogram(fn_readline);

    /* std.textio library
     * procedure writeline (file f: text; l: inout line);
     */
    std::list<InterfacePort*>*fn_writeline_args = new std::list<InterfacePort*>();
    fn_writeline_args->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
    fn_writeline_args->push_back(new InterfacePort(&primitive_STRING, PORT_IN));
    SubprogramBuiltin*fn_writeline = new SubprogramBuiltin(perm_string::literal("writeline"),
                                       perm_string::literal("$ivlh_writeline"),
                                       fn_writeline_args, NULL);
    register_std_subprogram(fn_writeline);

    /* function endline (file f: text) return boolean;
     */
    std::list<InterfacePort*>*fn_endfile_args = new std::list<InterfacePort*>();
    fn_endfile_args->push_back(new InterfacePort(&primitive_INTEGER, PORT_IN));
    SubprogramBuiltin*fn_endfile = new SubprogramBuiltin(perm_string::literal("endfile"),
                                       perm_string::literal("$feof"),
                                       fn_endfile_args, &type_BOOLEAN);
    register_std_subprogram(fn_endfile);
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
