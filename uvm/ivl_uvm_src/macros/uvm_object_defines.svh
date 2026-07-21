//------------------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
//   All Rights Reserved Worldwide
//
//   Licensed under the Apache License, Version 2.0 (the
//   "License"); you may not use this file except in
//   compliance with the License.  You may obtain a copy of
//   the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in
//   writing, software distributed under the License is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//   CONDITIONS OF ANY KIND, either express or implied.  See
//   the License for the specific language governing
//   permissions and limitations under the License.
//------------------------------------------------------------------------------

`ifndef UVM_OBJECT_DEFINES_SVH
`define UVM_OBJECT_DEFINES_SVH

`ifdef UVM_EMPTY_MACROS

`define uvm_field_utils_begin(T) 
`define uvm_field_utils_end 
`define uvm_object_utils(T) 
`define uvm_object_param_utils(T) 
`define uvm_object_utils_begin(T) 
`define uvm_object_param_utils_begin(T) 
`define uvm_object_utils_end
`define uvm_component_utils(T)
`define uvm_component_param_utils(T)
`define uvm_component_utils_begin(T)
`define uvm_component_param_utils_begin(T)
`define uvm_component_utils_end
`define uvm_field_int(ARG,FLAG)
`define uvm_field_real(ARG,FLAG)
`define uvm_field_enum(T,ARG,FLAG)
`define uvm_field_object(ARG,FLAG)
`define uvm_field_event(ARG,FLAG)
`define uvm_field_string(ARG,FLAG)
`define uvm_field_array_enum(ARG,FLAG)
`define uvm_field_array_int(ARG,FLAG)
`define uvm_field_sarray_int(ARG,FLAG)
`define uvm_field_sarray_enum(ARG,FLAG)
`define uvm_field_array_object(ARG,FLAG)
`define uvm_field_sarray_object(ARG,FLAG)
`define uvm_field_array_string(ARG,FLAG)
`define uvm_field_sarray_string(ARG,FLAG)
`define uvm_field_queue_enum(ARG,FLAG)
`define uvm_field_queue_int(ARG,FLAG)
`define uvm_field_queue_object(ARG,FLAG)
`define uvm_field_queue_string(ARG,FLAG)
`define uvm_field_aa_int_string(ARG, FLAG)
`define uvm_field_aa_string_string(ARG, FLAG)
`define uvm_field_aa_object_string(ARG, FLAG)
`define uvm_field_aa_int_int(ARG, FLAG)
`define uvm_field_aa_int_int(ARG, FLAG)
`define uvm_field_aa_int_int_unsigned(ARG, FLAG)
`define uvm_field_aa_int_integer(ARG, FLAG)
`define uvm_field_aa_int_integer_unsigned(ARG, FLAG)
`define uvm_field_aa_int_byte(ARG, FLAG)
`define uvm_field_aa_int_byte_unsigned(ARG, FLAG)
`define uvm_field_aa_int_shortint(ARG, FLAG)
`define uvm_field_aa_int_shortint_unsigned(ARG, FLAG)
`define uvm_field_aa_int_longint(ARG, FLAG)
`define uvm_field_aa_int_longint_unsigned(ARG, FLAG)
`define uvm_field_aa_int_key(KEY, ARG, FLAG)
`define uvm_field_aa_string_int(ARG, FLAG)
`define uvm_field_aa_object_int(ARG, FLAG)

`else

//------------------------------------------------------------------------------
//
// Title: Utility and Field Macros for Components and Objects
//
// Group: Utility Macros 
//
// The ~utils~ macros define the infrastructure needed to enable the
// object/component for correct factory operation. See <`uvm_object_utils> and
// <`uvm_component_utils> for details.
//
// A ~utils~ macro should be used inside ~every~ user-defined class that extends
// <uvm_object> directly or indirectly, including <uvm_sequence_item> and
// <uvm_component>.
//
// Below is an example usage of the ~utils~ macro for a user-defined object.
//
//|  class mydata extends uvm_object;
//| 
//|     `uvm_object_utils(mydata)
//|
//|     // declare data properties
//|
//|    function new(string name="mydata_inst");
//|      super.new(name);
//|    endfunction
//|
//|  endclass
//
// Below is an example usage of a ~utils~ macro for a user-defined component. 
//
//|  class my_comp extends uvm_component;
//| 
//|     `uvm_component_utils(my_comp)
//|
//|     // declare data properties
//|
//|    function new(string name, uvm_component parent=null);
//|      super.new(name,parent);
//|    endfunction
//|
//|  endclass
//
//------------------------------------------------------------------------------

// Define- UVM_NO_REGISTERED_CONVERTER
//
// if this symbol is defined all auto registration of the proxies to print resources
// are disabled and you only get the typename printed (printing the objects contents 
// either requires fill %p support or an appropriate proxy registered)
// 
`ifdef UVM_NO_DEPRECATED 
  `define UVM_NO_REGISTERED_CONVERTER
`endif


// Definitions for the user to use inside their derived data class declarations.

// MACRO: `uvm_field_utils_begin

// MACRO: `uvm_field_utils_end
//
// These macros form a block in which `uvm_field_* macros can be placed. 
// Used as
//
//|  `uvm_field_utils_begin(TYPE)
//|    `uvm_field_* macros here
//|  `uvm_field_utils_end
//
// 
// These macros do ~not~ perform factory registration nor implement the
// ~get_type_name~ and ~create~ methods. Use this form when you need custom
// implementations of these two methods, or when you are setting up field macros
// for an abstract class (i.e. virtual class).

`define uvm_field_utils_begin(T) \
   function void __m_uvm_field_automation (uvm_object tmp_data__, \
                                     int what__, \
                                     string str__); \
   begin \
     T local_data__; /* Used for copy and compare */ \
     typedef T ___local_type____; \
     string string_aa_key; /* Used for associative array lookups */ \
     uvm_object __current_scopes[$]; \
     if(what__ inside {UVM_SETINT,UVM_SETSTR,UVM_SETOBJ}) begin \
        if(__m_uvm_status_container.m_do_cycle_check(this)) begin \
            return; \
        end \
        else \
            __current_scopes=__m_uvm_status_container.m_uvm_cycle_scopes; \
     end \
     super.__m_uvm_field_automation(tmp_data__, what__, str__); \
     /* Type is verified by uvm_object::compare() */ \
     if(tmp_data__ != null) \
       /* Allow objects in same hierarchy to be copied/compared */ \
       if(!$cast(local_data__, tmp_data__)) return;

`define uvm_field_utils_end \
     if(what__ inside {UVM_SETINT,UVM_SETSTR,UVM_SETOBJ}) begin \
        // remove all scopes recorded (through super and other objects visited before) \
        void'(__current_scopes.pop_back()); \
        __m_uvm_status_container.m_uvm_cycle_scopes = __current_scopes; \
     end \
     end \
endfunction \

// MACRO: `uvm_object_utils

// MACRO: `uvm_object_param_utils

// MACRO: `uvm_object_utils_begin

// MACRO: `uvm_object_param_utils_begin

// MACRO: `uvm_object_utils_end
//
// <uvm_object>-based class declarations may contain one of the above forms of
// utility macros.
// 
// For simple objects with no field macros, use
//
//|  `uvm_object_utils(TYPE)
//    
// For simple objects with field macros, use
//
//|  `uvm_object_utils_begin(TYPE)
//|    `uvm_field_* macro invocations here
//|  `uvm_object_utils_end
//    
// For parameterized objects with no field macros, use
//
//|  `uvm_object_param_utils(TYPE)
//    
// For parameterized objects, with field macros, use
//
//|  `uvm_object_param_utils_begin(TYPE)
//|    `uvm_field_* macro invocations here
//|  `uvm_object_utils_end
//
// Simple (non-parameterized) objects use the uvm_object_utils* versions, which
// do the following:
//
// o Implements get_type_name, which returns TYPE as a string
//
// o Implements create, which allocates an object of type TYPE by calling its
//   constructor with no arguments. TYPE's constructor, if defined, must have
//   default values on all it arguments.
//
// o Registers the TYPE with the factory, using the string TYPE as the factory
//   lookup string for the type.
//
// o Implements the static get_type() method which returns a factory
//   proxy object for the type.
//
// o Implements the virtual get_object_type() method which works just like the
//   static get_type() method, but operates on an already allocated object.
//
// Parameterized classes must use the uvm_object_param_utils* versions. They
// differ from <`uvm_object_utils> only in that they do not supply a type name
// when registering the object with the factory. As such, name-based lookup with
// the factory for parameterized classes is not possible.
//
// The macros with _begin suffixes are the same as the non-suffixed versions
// except that they also start a block in which `uvm_field_* macros can be
// placed. The block must be terminated by `uvm_object_utils_end.
//
// Objects deriving from uvm_sequence must use the `uvm_sequence_* macros
// instead of these macros.  See <`uvm_sequence_utils> for details.

`define uvm_object_utils(T) \
  `uvm_object_utils_begin(T) \
  `uvm_object_utils_end

`define uvm_object_param_utils(T) \
  `uvm_object_param_utils_begin(T) \
  `uvm_object_utils_end

`define uvm_object_utils_begin(T) \
   `m_uvm_object_registry_internal(T,T)  \
   `m_uvm_object_create_func(T) \
   `m_uvm_get_type_name_func(T) \
   `uvm_field_utils_begin(T) 

`define uvm_object_param_utils_begin(T) \
   `m_uvm_object_registry_param(T)  \
   `m_uvm_object_create_func(T) \
   `uvm_field_utils_begin(T) 
       
`define uvm_object_utils_end \
     end \
   endfunction \


// MACRO: `uvm_component_utils

// MACRO: `uvm_component_param_utils

// MACRO: `uvm_component_utils_begin

// MACRO: `uvm_component_param_utils_begin

// MACRO: `uvm_component_end
//
// uvm_component-based class declarations may contain one of the above forms of
// utility macros.
//
// For simple components with no field macros, use
//
//|  `uvm_component_utils(TYPE)
//
// For simple components with field macros, use
//
//|  `uvm_component_utils_begin(TYPE)
//|    `uvm_field_* macro invocations here
//|  `uvm_component_utils_end
//
// For parameterized components with no field macros, use
//
//|  `uvm_component_param_utils(TYPE)
//
// For parameterized components with field macros, use
//
//|  `uvm_component_param_utils_begin(TYPE)
//|    `uvm_field_* macro invocations here
//|  `uvm_component_utils_end
//
// Simple (non-parameterized) components must use the uvm_components_utils*
// versions, which do the following:
//
// o Implements get_type_name, which returns TYPE as a string.
//
// o Implements create, which allocates a component of type TYPE using a two
//   argument constructor. TYPE's constructor must have a name and a parent
//   argument.
//
// o Registers the TYPE with the factory, using the string TYPE as the factory
//   lookup string for the type.
//
// o Implements the static get_type() method which returns a factory
//   proxy object for the type.
//
// o Implements the virtual get_object_type() method which works just like the
//   static get_type() method, but operates on an already allocated object.
//
// Parameterized classes must use the uvm_object_param_utils* versions. They
// differ from `uvm_object_utils only in that they do not supply a type name
// when registering the object with the factory. As such, name-based lookup with
// the factory for parameterized classes is not possible.
//
// The macros with _begin suffixes are the same as the non-suffixed versions
// except that they also start a block in which `uvm_field_* macros can be
// placed. The block must be terminated by `uvm_component_utils_end.
//

`define uvm_component_utils(T) \
   `m_uvm_component_registry_internal(T,T) \
   `m_uvm_get_type_name_func(T) \

`define uvm_component_param_utils(T) \
   `m_uvm_component_registry_param(T) \

   
`define uvm_component_utils_begin(T) \
   `uvm_component_utils(T) \
   `uvm_field_utils_begin(T) 

`define uvm_component_param_utils_begin(T) \
   `uvm_component_param_utils(T) \
   `uvm_field_utils_begin(T) 

`define uvm_component_utils_end \
     end \
   endfunction


// MACRO: `uvm_object_registry
//
// Register a uvm_object-based class with the factory
//
//| `uvm_object_registry(T,S)
//
// Registers a uvm_object-based class ~T~ and lookup
// string ~S~ with the factory. ~S~ typically is the
// name of the class in quotes. The <`uvm_object_utils>
// family of macros uses this macro.

`define uvm_object_registry(T,S) \
   typedef uvm_object_registry#(T,S) type_id; \
   static function type_id get_type(); \
     return type_id::get(); \
   endfunction \
   virtual function uvm_object_wrapper get_object_type(); \
     return type_id::get(); \
   endfunction 


// MACRO: `uvm_component_registry
//
// Registers a uvm_component-based class with the factory
//
//| `uvm_component_registry(T,S)
//
// Registers a uvm_component-based class ~T~ and lookup
// string ~S~ with the factory. ~S~ typically is the
// name of the class in quotes. The <`uvm_object_utils>
// family of macros uses this macro.

`define uvm_component_registry(T,S) \
   typedef uvm_component_registry #(T,S) type_id; \
   static function type_id get_type(); \
     return type_id::get(); \
   endfunction \
   virtual function uvm_object_wrapper get_object_type(); \
     return type_id::get(); \
   endfunction 


// uvm_new_func
// ------------

`define uvm_new_func \
  function new (string name, uvm_component parent); \
    super.new(name, parent); \
  endfunction


//-----------------------------------------------------------------------------
// INTERNAL MACROS - in support of *_utils macros -- do not use directly
//-----------------------------------------------------------------------------

// m_uvm_object_create_func
// ------------------------

`define m_uvm_object_create_func(T) \
   function uvm_object create (string name=""); \
     T tmp; \
`ifdef UVM_OBJECT_MUST_HAVE_CONSTRUCTOR \
     if (name=="") tmp = new(); \
     else tmp = new(name); \
`else \
     tmp = new(); \
     if (name!="") \
       tmp.set_name(name); \
`endif \
     return tmp; \
   endfunction


// m_uvm_get_type_name_func
// ----------------------

`define m_uvm_get_type_name_func(T) \
   const static string type_name = `"T`"; \
   virtual function string get_type_name (); \
     return type_name; \
   endfunction 


// m_uvm_object_registry_internal
// ------------------------------

//This is needed due to an issue in of passing down strings
//created by args to lower level macros.
`define m_uvm_object_registry_internal(T,S) \
   typedef uvm_object_registry#(T,`"S`") type_id; \
   static function type_id get_type(); \
     return type_id::get(); \
   endfunction \
   virtual function uvm_object_wrapper get_object_type(); \
     return type_id::get(); \
   endfunction 


// m_uvm_object_registry_param
// ---------------------------

`define m_uvm_object_registry_param(T) \
   typedef uvm_object_registry #(T) type_id; \
   static function type_id get_type(); \
     return type_id::get(); \
   endfunction \
   virtual function uvm_object_wrapper get_object_type(); \
     return type_id::get(); \
   endfunction 


// m_uvm_component_registry_internal
// ---------------------------------

//This is needed due to an issue in of passing down strings
//created by args to lower level macros.
`define m_uvm_component_registry_internal(T,S) \
   typedef uvm_component_registry #(T,`"S`") type_id; \
   static function type_id get_type(); \
     return type_id::get(); \
   endfunction \
   virtual function uvm_object_wrapper get_object_type(); \
     return type_id::get(); \
   endfunction

// versions of the uvm_component_registry macros to be used with
// parameterized classes

// m_uvm_component_registry_param
// ------------------------------

`define m_uvm_component_registry_param(T) \
   typedef uvm_component_registry #(T) type_id; \
   static function type_id get_type(); \
     return type_id::get(); \
   endfunction \
   virtual function uvm_object_wrapper get_object_type(); \
     return type_id::get(); \
   endfunction



//------------------------------------------------------------------------------
//
// Group: Field Macros
//
// The `uvm_field_*  macros are invoked inside of the `uvm_*_utils_begin and
// `uvm_*_utils_end macro blocks to form "automatic" implementations of the
// core data methods: copy, compare, pack, unpack, record, print, and sprint.
//
// By using the macros, you do not have to implement any of the do_* methods 
// inherited from <uvm_object>. However, be aware that the field macros expand
// into general inline code that is not as run-time efficient nor as flexible
// as direct implementions of the do_* methods. 
//
// Below is an example usage of the field macros for a sequence item. 
//
//|  class my_trans extends uvm_sequence_item;
//| 
//|    cmd_t  cmd;
//|    int    addr;
//|    int    data[$];
//|    my_ext ext;
//|    string str;
//|
//|    `uvm_object_utils_begin(my_trans)
//|      `uvm_field_enum     (cmd_t, cmd, UVM_ALL_ON)
//|      `uvm_field_int      (addr, UVM_ALL_ON)
//|      `uvm_field_queue_int(data, UVM_ALL_ON)
//|      `uvm_field_object   (ext,  UVM_ALL_ON)
//|      `uvm_field_string   (str,  UVM_ALL_ON)
//|    `uvm_object_utils_end
//|
//|    function new(string name="mydata_inst");
//|      super.new(name);
//|    endfunction
//|
//|  endclass
//
// Below is an example usage of the field macros for a component.
//
//|  class my_comp extends uvm_component;
//| 
//|    my_comp_cfg  cfg;
//|
//|    `uvm_component_utils_begin(my_comp)
//|      `uvm_field_object   (cfg,  UVM_ALL_ON)
//|    `uvm_object_utils_end
//|
//|    function new(string name="my_comp_inst", uvm_component parent=null);
//|      super.new(name);
//|    endfunction
//|
//|  endclass
//
// Each `uvm_field_* macro is named according to the particular data type it
// handles: integrals, strings, objects, queues, etc., and each has at least two
// arguments: ~ARG~ and ~FLAG~.
//
// ARG -  is the instance name of the variable, whose type must be compatible with
// the macro being invoked. In the example, class variable ~addr~ is an integral type,
// so we use the ~`uvm_field_int~ macro.
//
// FLAG - if set to ~UVM_ALL_ON~, as in the example, the ARG variable will be
// included in all data methods. If FLAG is set to something other than
// ~UVM_ALL_ON~ or ~UVM_DEFAULT~, it specifies which data method implementations will
// ~not~ include the given variable. Thus, if ~FLAG~ is specified as ~NO_COMPARE~,
// the ARG variable will not affect comparison operations, but it will be
// included in everything else.
//
// All possible values for ~FLAG~ are listed and described below. Multiple flag
// values can be bitwise ORed together (in most cases they may be added together
// as well, but care must be taken when using the + operator to ensure that the
// same bit is not added more than once).
//
//   UVM_ALL_ON     - Set all operations on.
//   UVM_DEFAULT    - This is the recommended set of flags to pass 
//                      to the field macros.  Currently, it enables
//                      all of the operations, making it functionally
//                      identical to ~UVM_ALL_ON~.  In the future 
//                      however, additional flags could be added with
//                      a recommended default value of ~off~.
//
//   UVM_NOCOPY     - Do not copy this field.
//   UVM_NOCOMPARE  - Do not compare this field.
//   UVM_NOPRINT    - Do not print this field.
//   UVM_NOPACK     - Do not pack or unpack this field.
//
//   UVM_REFERENCE  - For object types, operate only on the handle (e.g. no deep copy)
//
//   UVM_PHYSICAL   - Treat as a physical field. Use physical setting in
//                      policy class for this field.
//   UVM_ABSTRACT   - Treat as an abstract field. Use the abstract setting
//                      in the policy class for this field.
//   UVM_READONLY   - Do not allow setting of this field from the set_*_local
//                      methods or during <apply_config_settings> operation.
//
//
// A radix for printing and recording can be specified by OR'ing one of the
// following constants in the ~FLAG~ argument
//
//   UVM_BIN      - Print / record the field in binary (base-2).
//   UVM_DEC      - Print / record the field in decimal (base-10).
//   UVM_UNSIGNED - Print / record the field in unsigned decimal (base-10).
//   UVM_OCT      - Print / record the field in octal (base-8).
//   UVM_HEX      - Print / record the field in hexidecimal (base-16).
//   UVM_STRING   - Print / record the field in string format.
//   UVM_TIME     - Print / record the field in time format.
//
//   Radix settings for integral types. Hex is the default radix if none is
//   specified.
//
// A UVM component should ~not~ be specified using the `uvm_field_object macro
// unless its flag includes UVM_REFERENCE.  Otherwise, the field macro will 
// implement deep copy, which is an illegal operation for uvm_components.
// You will get a FATAL error if you tried to copy or clone an object containing
// a component handle that was registered with a field macro without the
// UVM_REFERENCE flag. You will also get duplicate entries when printing
// component topology, as this functionality is already provided by UVM. 
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Group: `uvm_field_* macros
//
// Macros that implement data operations for scalar properties.
//
//-----------------------------------------------------------------------------

// MACRO: `uvm_field_int
//
// Implements the data operations for any packed integral property.
//
//|  `uvm_field_int(ARG,FLAG)
//
// ~ARG~ is an integral property of the class, and ~FLAG~ is a bitwise OR of
// one or more flag settings as described in <Field Macros> above.

`define uvm_field_int(ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        begin \
          __m_uvm_status_container.do_field_check(`"ARG`", this); \
        end \
      UVM_COPY: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) ARG = local_data__.ARG; \
        end \
      UVM_COMPARE: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(ARG !== local_data__.ARG) begin \
               void'(__m_uvm_status_container.comparer.compare_field(`"ARG`", ARG, local_data__.ARG, $bits(ARG))); \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          if($bits(ARG) <= 64) __m_uvm_status_container.packer.pack_field_int(ARG, $bits(ARG)); \
          else __m_uvm_status_container.packer.pack_field(ARG, $bits(ARG)); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          if($bits(ARG) <= 64) ARG =  __m_uvm_status_container.packer.unpack_field_int($bits(ARG)); \
          else ARG = __m_uvm_status_container.packer.unpack_field($bits(ARG)); \
        end \
      UVM_RECORD: \
        `m_uvm_record_int(ARG, FLAG) \
      UVM_PRINT: \
        if(!((FLAG)&UVM_NOPRINT)) begin \
          __m_uvm_status_container.printer.print_int(`"ARG`", ARG, $bits(ARG), uvm_radix_enum'((FLAG)&UVM_RADIX)); \
        end \
      UVM_SETINT: \
        begin \
          bit matched; \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          matched = uvm_is_match(str__, __m_uvm_status_container.scope.get()); \
          if(matched) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            else begin \
              if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
              ARG = uvm_object::__m_uvm_status_container.bitstream; \
              uvm_object::__m_uvm_status_container.status = 1; \
            end \
          end \
          __m_uvm_status_container.scope.unset_arg(`"ARG`"); \
        end \
    endcase \
  end


// MACRO: `uvm_field_object
//
// Implements the data operations for an <uvm_object>-based property.
//
//|  `uvm_field_object(ARG,FLAG)
//
// ~ARG~ is an object property of the class, and ~FLAG~ is a bitwise OR of
// one or more flag settings as described in <Field Macros> above.

`define uvm_field_object(ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        __m_uvm_status_container.do_field_check(`"ARG`", this); \
      UVM_COPY: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) begin \
            if((FLAG)&UVM_REFERENCE || local_data__.ARG == null) ARG = local_data__.ARG; \
            else begin \
              uvm_object l_obj; \
              if(local_data__.ARG.get_name() == "") local_data__.ARG.set_name(`"ARG`"); \
              l_obj = local_data__.ARG.clone(); \
              if(l_obj == null) begin \
                `uvm_fatal("FAILCLN", $sformatf("Failure to clone %s.ARG, thus the variable will remain null.", local_data__.get_name())); \
              end \
              else begin \
                $cast(ARG, l_obj); \
                ARG.set_name(local_data__.ARG.get_name()); \
              end \
            end \
          end \
        end \
      UVM_COMPARE: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            void'(__m_uvm_status_container.comparer.compare_object(`"ARG`", ARG, local_data__.ARG)); \
            if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          if(((FLAG)&UVM_NOPACK) == 0 && ((FLAG)&UVM_REFERENCE) == 0) \
            __m_uvm_status_container.packer.pack_object(ARG); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          if(((FLAG)&UVM_NOPACK) == 0 && ((FLAG)&UVM_REFERENCE) == 0) \
            __m_uvm_status_container.packer.unpack_object(ARG); \
        end \
      UVM_RECORD: \
        `m_uvm_record_object(ARG,FLAG) \
      UVM_PRINT: \
        begin \
          if(!((FLAG)&UVM_NOPRINT)) begin \
            if(((FLAG)&UVM_REFERENCE) != 0) \
              __m_uvm_status_container.printer.print_object_header(`"ARG`", ARG); \
            else \
              __m_uvm_status_container.printer.print_object(`"ARG`", ARG); \
          end \
        end \
      UVM_SETINT: \
        begin \
          if((ARG != null) && (((FLAG)&UVM_READONLY)==0) && (((FLAG)&UVM_REFERENCE)==0)) begin \
            __m_uvm_status_container.scope.down(`"ARG`"); \
            ARG.__m_uvm_field_automation(null, UVM_SETINT, str__); \
            __m_uvm_status_container.scope.up(); \
          end \
        end \
      UVM_SETSTR: \
        begin \
          if((ARG != null) && (((FLAG)&UVM_READONLY)==0) && (((FLAG)&UVM_REFERENCE)==0)) begin \
            __m_uvm_status_container.scope.down(`"ARG`"); \
            ARG.__m_uvm_field_automation(null, UVM_SETSTR, str__); \
            __m_uvm_status_container.scope.up(); \
          end \
        end \
      UVM_SETOBJ: \
        begin \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            else begin \
              if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_object()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
              if($cast(ARG,uvm_object::__m_uvm_status_container.object)) \
                uvm_object::__m_uvm_status_container.status = 1; \
            end \
          end \
          else if(ARG!=null && ((FLAG)&UVM_READONLY) == 0) begin \
            int cnt; \
            //Only traverse if there is a possible match. \
            for(cnt=0; cnt<str__.len(); ++cnt) begin \
              if(str__[cnt] == "." || str__[cnt] == "*") break; \
            end \
            if(cnt!=str__.len()) begin \
              __m_uvm_status_container.scope.down(`"ARG`"); \
              ARG.__m_uvm_field_automation(null, UVM_SETOBJ, str__); \
              __m_uvm_status_container.scope.up(); \
            end \
          end \
        end \
    endcase \
  end


// MACRO: `uvm_field_string
//
// Implements the data operations for a string property.
//
//|  `uvm_field_string(ARG,FLAG)
//
// ~ARG~ is a string property of the class, and ~FLAG~ is a bitwise OR of
// one or more flag settings as described in <Field Macros> above.

`define uvm_field_string(ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        __m_uvm_status_container.do_field_check(`"ARG`", this); \
      UVM_COPY: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) ARG = local_data__.ARG; \
        end \
      UVM_COMPARE: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(ARG != local_data__.ARG) begin \
               void'(__m_uvm_status_container.comparer.compare_string(`"ARG`", ARG, local_data__.ARG)); \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          __m_uvm_status_container.packer.pack_string(ARG); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          ARG = __m_uvm_status_container.packer.unpack_string(); \
        end \
      UVM_RECORD: \
        `m_uvm_record_string(ARG, ARG, FLAG) \
      UVM_PRINT: \
        if(!((FLAG)&UVM_NOPRINT)) begin \
          __m_uvm_status_container.printer.print_string(`"ARG`", ARG); \
        end \
      UVM_SETSTR: \
        begin \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            else begin \
              if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_str()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
              ARG = uvm_object::__m_uvm_status_container.stringv; \
              __m_uvm_status_container.status = 1; \
            end \
          end \
      end \
    endcase \
  end


// MACRO: `uvm_field_enum
// 
// Implements the data operations for an enumerated property.
//
//|  `uvm_field_enum(T,ARG,FLAG)
//
// ~T~ is an enumerated _type_, ~ARG~ is an instance of that type, and
// ~FLAG~ is a bitwise OR of one or more flag settings as described in
// <Field Macros> above.

`define uvm_field_enum(T,ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        __m_uvm_status_container.do_field_check(`"ARG`", this); \
      UVM_COPY: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) ARG = local_data__.ARG; \
        end \
      UVM_COMPARE: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(ARG !== local_data__.ARG) begin \
               __m_uvm_status_container.scope.set_arg(`"ARG`"); \
               $swrite(__m_uvm_status_container.stringv, "lhs = %0s : rhs = %0s", \
                 ARG.name(), local_data__.ARG.name()); \
               __m_uvm_status_container.comparer.print_msg(__m_uvm_status_container.stringv); \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          __m_uvm_status_container.packer.pack_field(ARG, $bits(ARG)); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          ARG =  T'(__m_uvm_status_container.packer.unpack_field_int($bits(ARG))); \
        end \
      UVM_RECORD: \
        `m_uvm_record_string(ARG, ARG.name(), FLAG) \
      UVM_PRINT: \
        if(!((FLAG)&UVM_NOPRINT)) begin \
          __m_uvm_status_container.printer.print_generic(`"ARG`", `"T`", $bits(ARG), ARG.name()); \
        end \
      UVM_SETINT: \
        begin \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            else begin \
              if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
              ARG = T'(uvm_object::__m_uvm_status_container.bitstream); \
              __m_uvm_status_container.status = 1; \
            end \
          end \
      end \
    endcase \
  end


// MACRO: `uvm_field_real
//
// Implements the data operations for any real property.
//
//|  `uvm_field_real(ARG,FLAG)
//
// ~ARG~ is an real property of the class, and ~FLAG~ is a bitwise OR of
// one or more flag settings as described in <Field Macros> above.

`define uvm_field_real(ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        __m_uvm_status_container.do_field_check(`"ARG`", this); \
      UVM_COPY: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) ARG = local_data__.ARG; \
        end \
      UVM_COMPARE: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(ARG != local_data__.ARG) begin \
               void'(__m_uvm_status_container.comparer.compare_field_real(`"ARG`", ARG, local_data__.ARG)); \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          __m_uvm_status_container.packer.pack_field_int($realtobits(ARG), 64); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          ARG = $bitstoreal(__m_uvm_status_container.packer.unpack_field_int(64)); \
        end \
      UVM_RECORD: \
        if(!((FLAG)&UVM_NORECORD)) begin \
          __m_uvm_status_container.recorder.record_field_real(`"ARG`", ARG); \
        end \
      UVM_PRINT: \
        if(!((FLAG)&UVM_NOPRINT)) begin \
          __m_uvm_status_container.printer.print_real(`"ARG`", ARG); \
        end \
      UVM_SETINT: \
        begin \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            else begin \
              if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
              ARG = $bitstoreal(uvm_object::__m_uvm_status_container.bitstream); \
              __m_uvm_status_container.status = 1; \
            end \
          end \
      end \
    endcase \
  end


// MACRO: `uvm_field_event
//   
// Implements the data operations for an event property.
//
//|  `uvm_field_event(ARG,FLAG)
//
// ~ARG~ is an event property of the class, and ~FLAG~ is a bitwise OR of
// one or more flag settings as described in <Field Macros> above.

`define uvm_field_event(ARG,FLAG) \
  begin \
    case (what__) \
      UVM_COPY: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) ARG = local_data__.ARG; \
        end \
      UVM_COMPARE: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(ARG != local_data__.ARG) begin \
               __m_uvm_status_container.scope.down(`"ARG`"); \
               __m_uvm_status_container.comparer.print_msg(""); \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          // Events aren't packed or unpacked  \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
        end \
      UVM_RECORD: \
        begin \
          // Events are not recorded  \
        end \
      UVM_PRINT: \
        if(!((FLAG)&UVM_NOPRINT)) begin \
          __m_uvm_status_container.printer.print_generic(`"ARG`", "event", -1, ""); \
        end \
      UVM_SETINT: \
        begin \
          // Events are not configurable via set_config \
        end \
    endcase \
  end


//-----------------------------------------------------------------------------
// Group: `uvm_field_sarray_* macros
//                            
// Macros that implement data operations for one-dimensional static array
// properties.
//-----------------------------------------------------------------------------

// MACRO: `uvm_field_sarray_int
//
// Implements the data operations for a one-dimensional static array of
// integrals.
//
//|  `uvm_field_sarray_int(ARG,FLAG)
//
// ~ARG~ is a one-dimensional static array of integrals, and ~FLAG~
// is a bitwise OR of one or more flag settings as described in
// <Field Macros> above.

`define uvm_field_sarray_int(ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        __m_uvm_status_container.do_field_check(`"ARG`", this); \
      UVM_COPY: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) ARG = local_data__.ARG; \
        end \
      UVM_COMPARE: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(ARG !== local_data__.ARG) begin \
               if(__m_uvm_status_container.comparer.show_max == 1) begin \
                 __m_uvm_status_container.scope.set_arg(`"ARG`"); \
                 __m_uvm_status_container.comparer.print_msg(""); \
               end \
               else if(__m_uvm_status_container.comparer.show_max) begin \
                 foreach(ARG[i]) begin \
                   if(ARG[i] !== local_data__.ARG[i]) begin \
                     __m_uvm_status_container.scope.set_arg_element(`"ARG`",i); \
                     void'(__m_uvm_status_container.comparer.compare_field("", ARG[i], local_data__.ARG[i], $bits(ARG[i]))); \
                   end \
                 end \
               end \
               else if ((__m_uvm_status_container.comparer.physical&&((FLAG)&UVM_PHYSICAL)) || \
                        (__m_uvm_status_container.comparer.abstract&&((FLAG)&UVM_ABSTRACT)) || \
                        (!((FLAG)&UVM_PHYSICAL) && !((FLAG)&UVM_ABSTRACT)) ) \
                 __m_uvm_status_container.comparer.result++; \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          foreach(ARG[i])  \
            if($bits(ARG[i]) <= 64) __m_uvm_status_container.packer.pack_field_int(ARG[i], $bits(ARG[i])); \
            else __m_uvm_status_container.packer.pack_field(ARG[i], $bits(ARG[i])); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          foreach(ARG[i]) \
            if($bits(ARG[i]) <= 64) ARG[i] = __m_uvm_status_container.packer.unpack_field_int($bits(ARG[i])); \
            else ARG[i] = __m_uvm_status_container.packer.unpack_field($bits(ARG[i])); \
        end \
      UVM_RECORD: \
        `m_uvm_record_qda_int(ARG, FLAG, $size(ARG))  \
      UVM_PRINT: \
        if(!((FLAG)&UVM_NOPRINT)) begin \
          if(((FLAG)&UVM_NOPRINT) == 0) begin \
             `uvm_print_sarray_int3(ARG, uvm_radix_enum'((FLAG)&(UVM_RADIX)), \
                                   __m_uvm_status_container.printer) \
          end \
        end \
      UVM_SETINT: \
        begin \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            else begin \
              uvm_report_warning("RDONLY", $sformatf("%s: static arrays cannot be resized via configuraton.",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
          end \
          else if(!((FLAG)&UVM_READONLY)) begin \
            foreach(ARG[i]) begin \
              __m_uvm_status_container.scope.set_arg_element(`"ARG`",i); \
              if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
                if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
                ARG[i] =  uvm_object::__m_uvm_status_container.bitstream; \
                __m_uvm_status_container.status = 1; \
              end \
            end \
          end \
        end \
    endcase \
  end


// MACRO: `uvm_field_sarray_object
//
// Implements the data operations for a one-dimensional static array of
// <uvm_object>-based objects.
//
//|  `uvm_field_sarray_object(ARG,FLAG)
//
// ~ARG~ is a one-dimensional static array of <uvm_object>-based objects,
// and ~FLAG~ is a bitwise OR of one or more flag settings as described in
// <Field Macros> above.

`define uvm_field_sarray_object(ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        __m_uvm_status_container.do_field_check(`"ARG`", this); \
      UVM_COPY: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) begin \
            if(((FLAG)&UVM_REFERENCE)) \
              ARG = local_data__.ARG; \
            else \
              foreach(ARG[i]) begin \
                if(ARG[i] != null && local_data__.ARG[i] != null) \
                  ARG[i].copy(local_data__.ARG[i]); \
                else if(ARG[i] == null && local_data__.ARG[i] != null) \
                  $cast(ARG[i], local_data__.ARG[i].clone()); \
                else \
                  ARG[i] = null; \
              end \
          end \
        end \
      UVM_COMPARE: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(((FLAG)&UVM_REFERENCE) && (__m_uvm_status_container.comparer.show_max <= 1) && (ARG !== local_data__.ARG) ) begin \
               if(__m_uvm_status_container.comparer.show_max == 1) begin \
                 __m_uvm_status_container.scope.set_arg(`"ARG`"); \
                 __m_uvm_status_container.comparer.print_msg(""); \
               end \
               else if ((__m_uvm_status_container.comparer.physical&&((FLAG)&UVM_PHYSICAL)) || \
                        (__m_uvm_status_container.comparer.abstract&&((FLAG)&UVM_ABSTRACT)) || \
                        (!((FLAG)&UVM_PHYSICAL) && !((FLAG)&UVM_ABSTRACT)) ) \
                 __m_uvm_status_container.comparer.result++; \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
            else begin \
              string s; \
              foreach(ARG[i]) begin \
                if(ARG[i] != null && local_data__.ARG[i] != null) begin \
                  $swrite(s,`"ARG[%0d]`",i); \
                  void'(__m_uvm_status_container.comparer.compare_object(s, ARG[i], local_data__.ARG[i])); \
                end \
                if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
              end \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          foreach(ARG[i])  \
            __m_uvm_status_container.packer.pack_object(ARG[i]); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          foreach(ARG[i]) \
            __m_uvm_status_container.packer.unpack_object(ARG[i]); \
        end \
      UVM_RECORD: \
        `m_uvm_record_qda_object(ARG,FLAG,$size(ARG)) \
      UVM_PRINT: \
        begin \
          if(((FLAG)&UVM_NOPRINT) == 0) begin \
             `uvm_print_sarray_object3(ARG, __m_uvm_status_container.printer, FLAG) \
          end \
        end \
      UVM_SETINT: \
        begin \
          string s; \
          if(!((FLAG)&UVM_READONLY)) begin \
            foreach(ARG[i]) begin \
              $swrite(s,`"ARG[%0d]`",i); \
              __m_uvm_status_container.scope.set_arg(s); \
              if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
                if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_object()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
                if($cast(ARG[i],uvm_object::__m_uvm_status_container.object)) \
                  uvm_object::__m_uvm_status_container.status = 1; \
              end \
              else if(ARG[i]!=null && !((FLAG)&UVM_REFERENCE)) begin \
                int cnt; \
                //Only traverse if there is a possible match. \
                for(cnt=0; cnt<str__.len(); ++cnt) begin \
                  if(str__[cnt] == "." || str__[cnt] == "*") break; \
                end \
                if(cnt!=str__.len()) begin \
                  __m_uvm_status_container.scope.down(s); \
                  ARG[i].__m_uvm_field_automation(null, UVM_SETINT, str__); \
                  __m_uvm_status_container.scope.up(); \
                end \
              end \
            end \
          end \
        end \
      UVM_SETSTR: \
        begin \
          string s; \
          if(!((FLAG)&UVM_READONLY)) begin \
            foreach(ARG[i]) begin \
              $swrite(s,`"ARG[%0d]`",i); \
              __m_uvm_status_container.scope.set_arg(s); \
              if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
                if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_object()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
                if($cast(ARG[i],uvm_object::__m_uvm_status_container.object)) \
                  uvm_object::__m_uvm_status_container.status = 1; \
              end \
              else if(ARG[i]!=null && !((FLAG)&UVM_REFERENCE)) begin \
                int cnt; \
                //Only traverse if there is a possible match. \
                for(cnt=0; cnt<str__.len(); ++cnt) begin \
                  if(str__[cnt] == "." || str__[cnt] == "*") break; \
                end \
                if(cnt!=str__.len()) begin \
                  __m_uvm_status_container.scope.down(s); \
                  ARG[i].__m_uvm_field_automation(null, UVM_SETSTR, str__); \
                  __m_uvm_status_container.scope.up(); \
                end \
              end \
            end \
          end \
        end \
      UVM_SETOBJ: \
        begin \
          string s; \
          if(!((FLAG)&UVM_READONLY)) begin \
            foreach(ARG[i]) begin \
              $swrite(s,`"ARG[%0d]`",i); \
              __m_uvm_status_container.scope.set_arg(s); \
              if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
                if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_object()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
                if($cast(ARG[i],uvm_object::__m_uvm_status_container.object)) \
                  uvm_object::__m_uvm_status_container.status = 1; \
              end \
              else if(ARG[i]!=null && !((FLAG)&UVM_REFERENCE)) begin \
                int cnt; \
                //Only traverse if there is a possible match. \
                for(cnt=0; cnt<str__.len(); ++cnt) begin \
                  if(str__[cnt] == "." || str__[cnt] == "*") break; \
                end \
                if(cnt!=str__.len()) begin \
                  __m_uvm_status_container.scope.down(s); \
                  ARG[i].__m_uvm_field_automation(null, UVM_SETOBJ, str__); \
                  __m_uvm_status_container.scope.up(); \
                end \
              end \
            end \
          end \
        end \
    endcase \
  end


// MACRO: `uvm_field_sarray_string
//
// Implements the data operations for a one-dimensional static array of
// strings.
//
//|  `uvm_field_sarray_string(ARG,FLAG)
//
// ~ARG~ is a one-dimensional static array of strings, and ~FLAG~ is a bitwise
// OR of one or more flag settings as described in <Field Macros> above.

`define uvm_field_sarray_string(ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        __m_uvm_status_container.do_field_check(`"ARG`", this); \
      UVM_COPY: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) ARG = local_data__.ARG; \
        end \
      UVM_COMPARE: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(ARG != local_data__.ARG) begin \
               if(__m_uvm_status_container.comparer.show_max == 1) begin \
                 __m_uvm_status_container.scope.set_arg(`"ARG`"); \
                 __m_uvm_status_container.comparer.print_msg(""); \
               end \
               else if(__m_uvm_status_container.comparer.show_max) begin \
                 foreach(ARG[i]) begin \
                   if(ARG[i] != local_data__.ARG[i]) begin \
                     __m_uvm_status_container.scope.set_arg_element(`"ARG`",i); \
                     void'(__m_uvm_status_container.comparer.compare_string("", ARG[i], local_data__.ARG[i])); \
                   end \
                 end \
               end \
               else if ((__m_uvm_status_container.comparer.physical&&((FLAG)&UVM_PHYSICAL)) || \
                        (__m_uvm_status_container.comparer.abstract&&((FLAG)&UVM_ABSTRACT)) || \
                        (!((FLAG)&UVM_PHYSICAL) && !((FLAG)&UVM_ABSTRACT)) ) \
                 __m_uvm_status_container.comparer.result++; \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          foreach(ARG[i])  \
            __m_uvm_status_container.packer.pack_string(ARG[i]); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          foreach(ARG[i]) \
            ARG[i] = __m_uvm_status_container.packer.unpack_string(); \
        end \
      UVM_RECORD: \
        begin \
          /* Issue with $size for sarray with strings */ \
          int sz; foreach(ARG[i]) sz=i; \
          `m_uvm_record_qda_string(ARG, FLAG, sz) \
        end \
      UVM_PRINT: \
        begin \
          if(((FLAG)&UVM_NOPRINT) == 0) begin \
             `uvm_print_sarray_string2(ARG, __m_uvm_status_container.printer) \
          end \
        end \
      UVM_SETSTR: \
        begin \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            else begin \
              uvm_report_warning("RDONLY", {__m_uvm_status_container.get_full_scope_arg(), \
              ": static arrays cannot be resized via configuraton."}, UVM_NONE); \
            end \
          end \
          else if(!((FLAG)&UVM_READONLY)) begin \
            foreach(ARG[i]) begin \
              __m_uvm_status_container.scope.set_arg_element(`"ARG`",i); \
              if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
                if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
                ARG[i] =  uvm_object::__m_uvm_status_container.stringv; \
                __m_uvm_status_container.status = 1; \
              end \
            end \
          end \
        end \
    endcase \
  end


// MACRO: `uvm_field_sarray_enum
//
// Implements the data operations for a one-dimensional static array of
// enums.
//
//|  `uvm_field_sarray_enum(T,ARG,FLAG)
//
// ~T~ is a one-dimensional dynamic array of enums _type_, ~ARG~ is an
// instance of that type, and ~FLAG~ is a bitwise OR of one or more flag
// settings as described in <Field Macros> above.

`define uvm_field_sarray_enum(T,ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        __m_uvm_status_container.do_field_check(`"ARG`", this); \
      UVM_COPY: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) ARG = local_data__.ARG; \
        end \
      UVM_COMPARE: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(ARG !== local_data__.ARG) begin \
               if(__m_uvm_status_container.comparer.show_max == 1) begin \
                 __m_uvm_status_container.scope.set_arg(`"ARG`"); \
                 __m_uvm_status_container.comparer.print_msg(""); \
               end \
               else if(__m_uvm_status_container.comparer.show_max) begin \
                 foreach(ARG[i]) begin \
                   if(ARG[i] !== local_data__.ARG[i]) begin \
                     __m_uvm_status_container.scope.set_arg_element(`"ARG`",i); \
                     $swrite(__m_uvm_status_container.stringv, "lhs = %0s : rhs = %0s", \
                       ARG[i].name(), local_data__.ARG[i].name()); \
                     __m_uvm_status_container.comparer.print_msg(__m_uvm_status_container.stringv); \
                     if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
                   end \
                 end \
               end \
               else if ((__m_uvm_status_container.comparer.physical&&((FLAG)&UVM_PHYSICAL)) || \
                        (__m_uvm_status_container.comparer.abstract&&((FLAG)&UVM_ABSTRACT)) || \
                        (!((FLAG)&UVM_PHYSICAL) && !((FLAG)&UVM_ABSTRACT)) ) \
                 __m_uvm_status_container.comparer.result++; \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          foreach(ARG[i])  \
            __m_uvm_status_container.packer.pack_field_int(int'(ARG[i]), $bits(ARG[i])); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          foreach(ARG[i]) \
            ARG[i] = T'(__m_uvm_status_container.packer.unpack_field_int($bits(ARG[i]))); \
        end \
      UVM_RECORD: \
        `m_uvm_record_qda_enum(ARG, FLAG, $size(ARG)) \
      UVM_PRINT: \
        begin \
          if(((FLAG)&UVM_NOPRINT) == 0) begin \
             `uvm_print_qda_enum(ARG, __m_uvm_status_container.printer, array, T) \
          end \
        end \
      UVM_SETINT: \
        begin \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            else begin \
              uvm_report_warning("RDONLY", $sformatf("%s: static arrays cannot be resized via configuraton.",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
          end \
          else if(!((FLAG)&UVM_READONLY)) begin \
            foreach(ARG[i]) begin \
              __m_uvm_status_container.scope.set_arg_element(`"ARG`",i); \
              if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
                if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
                ARG[i] =  T'(uvm_object::__m_uvm_status_container.bitstream); \
                __m_uvm_status_container.status = 1; \
              end \
            end \
          end \
        end \
    endcase \
  end



//-----------------------------------------------------------------------------
// Group: `uvm_field_array_* macros
//
// Macros that implement data operations for one-dimensional dynamic array
// properties.
//
// Implementation note:
// lines flagged with empty multi-line comments, /**/, are not needed or need
// to be different for fixed arrays, which can not be resized. Fixed arrays 
// do not need to pack/unpack their size either, because their size is known;
// wouldn't hurt though if it allowed code consolidation. Unpacking would
// necessarily be different. */
// 
//-----------------------------------------------------------------------------

// M_UVM_QUEUE_RESIZE
// ------------------

`define M_UVM_QUEUE_RESIZE(ARG,VAL) \
  while(ARG.size()<sz) ARG.push_back(VAL); \
  while(ARG.size()>sz) void'(ARG.pop_front()); \


// M_UVM_ARRAY_RESIZE
// ------------------

`define M_UVM_ARRAY_RESIZE(ARG,VAL) \
  ARG = new[sz](ARG); \


// M_UVM_SARRAY_RESIZE
// -------------------

`define M_UVM_SARRAY_RESIZE(ARG,VAL) \
  /* fixed arrays can not be resized; do nothing */


// M_UVM_FIELD_QDA_INT
// -------------------

`define M_UVM_FIELD_QDA_INT(TYPE,ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        __m_uvm_status_container.do_field_check(`"ARG`", this); \
      UVM_COPY: \
        begin \
          if (local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) ARG = local_data__.ARG; \
        end \
      UVM_COMPARE: \
        begin \
          if (local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(ARG !== local_data__.ARG) begin \
               if(__m_uvm_status_container.comparer.show_max == 1) begin \
                 __m_uvm_status_container.scope.set_arg(`"ARG`"); \
                 __m_uvm_status_container.comparer.print_msg(""); \
               end \
               else if(__m_uvm_status_container.comparer.show_max) begin \
                 /**/ if(ARG.size() != local_data__.ARG.size()) begin \
                 /**/   void'(__m_uvm_status_container.comparer.compare_field(`"ARG``.size`", ARG.size(), local_data__.ARG.size(), 32)); \
                 /**/ end \
                 else begin \
                   foreach(ARG[i]) begin \
                     if(ARG[i] !== local_data__.ARG[i]) begin \
                       __m_uvm_status_container.scope.set_arg_element(`"ARG`",i); \
                       void'(__m_uvm_status_container.comparer.compare_field("", ARG[i], local_data__.ARG[i], $bits(ARG[i]))); \
                     end \
                   end \
                 end \
               end \
               else if ((__m_uvm_status_container.comparer.physical&&((FLAG)&UVM_PHYSICAL)) || \
                        (__m_uvm_status_container.comparer.abstract&&((FLAG)&UVM_ABSTRACT)) || \
                        (!((FLAG)&UVM_PHYSICAL) && !((FLAG)&UVM_ABSTRACT)) ) \
                 __m_uvm_status_container.comparer.result++; \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          /**/ if(__m_uvm_status_container.packer.use_metadata) __m_uvm_status_container.packer.pack_field_int(ARG.size(), 32); \
          foreach(ARG[i])  \
            if($bits(ARG[i]) <= 64) __m_uvm_status_container.packer.pack_field_int(ARG[i], $bits(ARG[i])); \
            else __m_uvm_status_container.packer.pack_field(ARG[i], $bits(ARG[i])); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          /**/ int sz = ARG.size(); \
          /**/ if(__m_uvm_status_container.packer.use_metadata) sz = __m_uvm_status_container.packer.unpack_field_int(32); \
          if(sz != ARG.size()) begin \
          `M_UVM_``TYPE``_RESIZE (ARG,0) \
          end \
          foreach(ARG[i]) \
            if($bits(ARG[i]) <= 64) ARG[i] = __m_uvm_status_container.packer.unpack_field_int($bits(ARG[i])); \
            else ARG[i] = __m_uvm_status_container.packer.unpack_field($bits(ARG[i])); \
        end \
      UVM_RECORD: \
        `m_uvm_record_qda_int(ARG, FLAG, ARG.size()) \
      UVM_PRINT: \
        begin \
          if(((FLAG)&UVM_NOPRINT) == 0) begin \
             `uvm_print_array_int3(ARG, uvm_radix_enum'((FLAG)&(UVM_RADIX)), \
                                   __m_uvm_status_container.printer) \
          end \
        end \
      UVM_SETINT: \
        begin \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            /**/ else begin \
            /**/   int sz =  uvm_object::__m_uvm_status_container.bitstream; \
            /**/   if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
            /**/   if(ARG.size() !=  sz) begin \
            /**/     `M_UVM_``TYPE``_RESIZE(ARG,0) \
            /**/   end \
            /**/   __m_uvm_status_container.status = 1; \
            /**/ end \
          end \
          else if(!((FLAG)&UVM_READONLY)) begin \
            bit wildcard_index__; \
            int index__; \
            index__ = uvm_get_array_index_int(str__, wildcard_index__); \
            if(uvm_is_array(str__)  && (index__ != -1)) begin\
              if(wildcard_index__) begin \
                for(index__=0; index__<ARG.size(); ++index__) begin \
                  if(uvm_is_match(str__, {__m_uvm_status_container.scope.get_arg(),$sformatf("[%0d]", index__)})) begin \
                    if (__m_uvm_status_container.print_matches) \
                      uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg(), $sformatf("[%0d]",index__)}, UVM_LOW); \
                    ARG[index__] = uvm_object::__m_uvm_status_container.bitstream; \
                    __m_uvm_status_container.status = 1; \
                  end \
                end \
              end \
              else if(uvm_is_match(str__, {__m_uvm_status_container.scope.get_arg(),$sformatf("[%0d]", index__)})) begin \
                if(index__+1 > ARG.size()) begin \
                  int sz = index__; \
                  int tmp__; \
                  `M_UVM_``TYPE``_RESIZE(ARG,tmp__) \
                end \
                if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
                ARG[index__] =  uvm_object::__m_uvm_status_container.bitstream; \
                __m_uvm_status_container.status = 1; \
              end \
            end \
          end \
        end \
    endcase \
  end


// MACRO: `uvm_field_array_int
//
// Implements the data operations for a one-dimensional dynamic array of
// integrals.
//
//|  `uvm_field_array_int(ARG,FLAG)
//
// ~ARG~ is a one-dimensional dynamic array of integrals,
// and ~FLAG~ is a bitwise OR of one or more flag settings as described in
// <Field Macros> above.

`define uvm_field_array_int(ARG,FLAG) \
   `M_UVM_FIELD_QDA_INT(ARRAY,ARG,FLAG) 


// MACRO: `uvm_field_array_object
//
// Implements the data operations for a one-dimensional dynamic array
// of <uvm_object>-based objects.
//
//|  `uvm_field_array_object(ARG,FLAG)
//
// ~ARG~ is a one-dimensional dynamic array of <uvm_object>-based objects,
// and ~FLAG~ is a bitwise OR of one or more flag settings as described in
// <Field Macros> above.

`define uvm_field_array_object(ARG,FLAG) \
  `M_UVM_FIELD_QDA_OBJECT(ARRAY,ARG,FLAG)

`define M_UVM_FIELD_QDA_OBJECT(TYPE,ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        __m_uvm_status_container.do_field_check(`"ARG`", this); \
      UVM_COPY: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) begin \
            if(((FLAG)&UVM_REFERENCE)) \
              ARG = local_data__.ARG; \
            else begin \
              int sz = local_data__.ARG.size(); \
              `M_UVM_``TYPE``_RESIZE(ARG,null) \
              foreach(ARG[i]) begin \
                if(ARG[i] != null && local_data__.ARG[i] != null) \
                  ARG[i].copy(local_data__.ARG[i]); \
                else if(ARG[i] == null && local_data__.ARG[i] != null) \
                  $cast(ARG[i], local_data__.ARG[i].clone()); \
                else \
                  ARG[i] = null; \
              end \
            end \
          end \
        end \
      UVM_COMPARE: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(((FLAG)&UVM_REFERENCE) && (__m_uvm_status_container.comparer.show_max <= 1) && (ARG !== local_data__.ARG) ) begin \
               if(__m_uvm_status_container.comparer.show_max == 1) begin \
                 __m_uvm_status_container.scope.set_arg(`"ARG`"); \
                 __m_uvm_status_container.comparer.print_msg(""); \
               end \
               else if ((__m_uvm_status_container.comparer.physical&&((FLAG)&UVM_PHYSICAL)) || \
                        (__m_uvm_status_container.comparer.abstract&&((FLAG)&UVM_ABSTRACT)) || \
                        (!((FLAG)&UVM_PHYSICAL) && !((FLAG)&UVM_ABSTRACT)) ) \
                 __m_uvm_status_container.comparer.result++; \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
            else begin \
              string s; \
              if(ARG.size() != local_data__.ARG.size()) begin \
                if(__m_uvm_status_container.comparer.show_max == 1) begin \
                  __m_uvm_status_container.scope.set_arg(`"ARG`"); \
                  __m_uvm_status_container.comparer.print_msg($sformatf("size mismatch: lhs: %0d  rhs: %0d", ARG.size(), local_data__.ARG.size())); \
                end \
              end \
              for(int i=0; i<ARG.size() && i<local_data__.ARG.size(); ++i) begin \
                if(ARG[i] != null && local_data__.ARG[i] != null) begin \
                  $swrite(s,`"ARG[%0d]`",i); \
                  void'(__m_uvm_status_container.comparer.compare_object(s, ARG[i], local_data__.ARG[i])); \
                end \
                if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
              end \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          if(__m_uvm_status_container.packer.use_metadata) __m_uvm_status_container.packer.pack_field_int(ARG.size(), 32); \
          foreach(ARG[i])  \
            __m_uvm_status_container.packer.pack_object(ARG[i]); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          int sz = ARG.size(); \
          if(__m_uvm_status_container.packer.use_metadata) sz = __m_uvm_status_container.packer.unpack_field_int(32); \
          if(sz != ARG.size()) begin \
            `M_UVM_``TYPE``_RESIZE(ARG,null) \
          end \
          foreach(ARG[i]) \
            __m_uvm_status_container.packer.unpack_object(ARG[i]); \
        end \
      UVM_RECORD: \
        `m_uvm_record_qda_object(ARG,FLAG,ARG.size()) \
      UVM_PRINT: \
        begin \
          if(((FLAG)&UVM_NOPRINT) == 0) begin \
             `uvm_print_array_object3(ARG, __m_uvm_status_container.printer,FLAG) \
          end \
        end \
      UVM_SETINT: \
        begin \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            else begin \
              int sz =  uvm_object::__m_uvm_status_container.bitstream; \
              if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
              if(ARG.size() !=  sz) begin \
                `M_UVM_``TYPE``_RESIZE(ARG,null) \
              end \
              __m_uvm_status_container.status = 1; \
            end \
          end \
          else if(!((FLAG)&UVM_READONLY)) begin \
            foreach(ARG[i]) begin \
              string s; \
              $swrite(s,`"ARG[%0d]`",i); \
              __m_uvm_status_container.scope.set_arg(s); \
              if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
		 uvm_report_warning("STRMTC", {"set_int()", ": Match ignored for string ", str__, ". Cannot set object to int value."}); \
              end \
              else if(ARG[i]!=null && !((FLAG)&UVM_REFERENCE)) begin \
                int cnt; \
                //Only traverse if there is a possible match. \
                for(cnt=0; cnt<str__.len(); ++cnt) begin \
                  if(str__[cnt] == "." || str__[cnt] == "*") break; \
                end \
                if(cnt!=str__.len()) begin \
                  __m_uvm_status_container.scope.down(s); \
                  ARG[i].__m_uvm_field_automation(null, UVM_SETINT, str__); \
                  __m_uvm_status_container.scope.up(); \
                end \
              end \
            end \
          end \
        end \
      UVM_SETSTR: \
        begin \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            else begin \
                  uvm_report_warning("STRMTC", {"set_str()", ": Match ignored for string ", str__, ". Cannot set array of objects to string value."}); \
            end \
          end \
          else if(!((FLAG)&UVM_READONLY)) begin \
            foreach(ARG[i]) begin \
              string s; \
              $swrite(s,`"ARG[%0d]`",i); \
              __m_uvm_status_container.scope.set_arg(s); \
              if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
                  uvm_report_warning("STRMTC", {"set_str()", ": Match ignored for string ", str__, ". Cannot set object to string value."}); \
              end \
              else if(ARG[i]!=null && !((FLAG)&UVM_REFERENCE)) begin \
                int cnt; \
                //Only traverse if there is a possible match. \
                for(cnt=0; cnt<str__.len(); ++cnt) begin \
                  if(str__[cnt] == "." || str__[cnt] == "*") break; \
                end \
                if(cnt!=str__.len()) begin \
                  __m_uvm_status_container.scope.down(s); \
                  ARG[i].__m_uvm_field_automation(null, UVM_SETSTR, str__); \
                  __m_uvm_status_container.scope.up(); \
                end \
              end \
            end \
          end \
        end \
      UVM_SETOBJ: \
        begin \
          string s; \
          if(!((FLAG)&UVM_READONLY)) begin \
            bit wildcard_index__; \
            int index__; \
            __m_uvm_status_container.scope.set_arg(`"ARG`"); \
            index__ = uvm_get_array_index_int(str__, wildcard_index__); \
            if(uvm_is_array(str__)  && (index__ != -1)) begin\
              if(wildcard_index__) begin \
                for(index__=0; index__<ARG.size(); ++index__) begin \
                  if(uvm_is_match(str__, {__m_uvm_status_container.scope.get_arg(),$sformatf("[%0d]", index__)})) begin \
                    if (__m_uvm_status_container.print_matches) \
                      uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg(), $sformatf("[%0d]",index__)}, UVM_LOW); \
                    $cast(ARG[index__], uvm_object::__m_uvm_status_container.object); \
                    __m_uvm_status_container.status = 1; \
                  end \
                end \
              end \
              else if(uvm_is_match(str__, {__m_uvm_status_container.get_full_scope_arg(),$sformatf("[%0d]", index__)})) begin \
                if(index__+1 > ARG.size()) begin \
                  int sz = index__+1; \
                  `M_UVM_``TYPE``_RESIZE(ARG,null) \
                end \
                if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
                $cast(ARG[index__],  uvm_object::__m_uvm_status_container.object); \
                __m_uvm_status_container.status = 1; \
              end \
            end \
            else if(!((FLAG)&UVM_REFERENCE)) begin \
              int cnt; \
              foreach(ARG[i]) begin \
                if (ARG[i]!=null) begin \
                  string s; \
                  $swrite(s,`"ARG[%0d]`",i); \
                //Only traverse if there is a possible match. \
                for(cnt=0; cnt<str__.len(); ++cnt) begin \
                  if(str__[cnt] == "." || str__[cnt] == "*") break; \
                end \
                if(cnt!=str__.len()) begin \
                  __m_uvm_status_container.scope.down(s); \
                  ARG[i].__m_uvm_field_automation(null, UVM_SETOBJ, str__); \
                  __m_uvm_status_container.scope.up(); \
                end \
              end \
            end \
          end \
        end \
        end \
    endcase \
  end 


// MACRO: `uvm_field_array_string
//
// Implements the data operations for a one-dimensional dynamic array 
// of strings.
//
//|  `uvm_field_array_string(ARG,FLAG)
//
// ~ARG~ is a one-dimensional dynamic array of strings, and ~FLAG~ is a bitwise
// OR of one or more flag settings as described in <Field Macros> above.

`define uvm_field_array_string(ARG,FLAG) \
  `M_UVM_FIELD_QDA_STRING(ARRAY,ARG,FLAG)

`define M_UVM_FIELD_QDA_STRING(TYPE,ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        __m_uvm_status_container.do_field_check(`"ARG`", this); \
      UVM_COPY: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOPY)) ARG = local_data__.ARG; \
        end \
      UVM_COMPARE: \
        begin \
          if(local_data__ == null) return; \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(ARG != local_data__.ARG) begin \
               if(__m_uvm_status_container.comparer.show_max == 1) begin \
                 __m_uvm_status_container.scope.set_arg(`"ARG`"); \
                 __m_uvm_status_container.comparer.print_msg(""); \
               end \
               else if(__m_uvm_status_container.comparer.show_max) begin \
                 if(ARG.size() != local_data__.ARG.size()) begin \
                   void'(__m_uvm_status_container.comparer.compare_field(`"ARG``.size`", ARG.size(), local_data__.ARG.size(), 32)); \
                 end \
                 else begin \
                   foreach(ARG[i]) begin \
                     if(ARG[i] != local_data__.ARG[i]) begin \
                       __m_uvm_status_container.scope.set_arg_element(`"ARG`",i); \
                       void'(__m_uvm_status_container.comparer.compare_string("", ARG[i], local_data__.ARG[i])); \
                     end \
                   end \
                 end \
               end \
               else if ((__m_uvm_status_container.comparer.physical&&((FLAG)&UVM_PHYSICAL)) || \
                        (__m_uvm_status_container.comparer.abstract&&((FLAG)&UVM_ABSTRACT)) || \
                        (!((FLAG)&UVM_PHYSICAL) && !((FLAG)&UVM_ABSTRACT)) ) \
                 __m_uvm_status_container.comparer.result++; \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          if(__m_uvm_status_container.packer.use_metadata) __m_uvm_status_container.packer.pack_field_int(ARG.size(), 32); \
          foreach(ARG[i])  \
            __m_uvm_status_container.packer.pack_string(ARG[i]); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          int sz = ARG.size(); \
          if(__m_uvm_status_container.packer.use_metadata) sz = __m_uvm_status_container.packer.unpack_field_int(32); \
          if(sz != ARG.size()) begin \
            `M_UVM_``TYPE``_RESIZE(ARG,"") \
          end \
          foreach(ARG[i]) \
            ARG[i] = __m_uvm_status_container.packer.unpack_string(); \
        end \
      UVM_RECORD: \
        `m_uvm_record_qda_string(ARG,FLAG,ARG.size()) \
      UVM_PRINT: \
        begin \
          if(((FLAG)&UVM_NOPRINT) == 0) begin \
             `uvm_print_array_string2(ARG, __m_uvm_status_container.printer) \
          end \
        end \
      UVM_SETINT: \
        begin \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            else begin \
              int sz =  uvm_object::__m_uvm_status_container.bitstream; \
              if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
              if(ARG.size() !=  sz) begin \
                `M_UVM_``TYPE``_RESIZE(ARG,"") \
              end \
              __m_uvm_status_container.status = 1; \
            end \
          end \
        end \
      UVM_SETSTR: \
        begin \
          if(!((FLAG)&UVM_READONLY)) begin \
            bit wildcard_index__; \
            int index__; \
            __m_uvm_status_container.scope.set_arg(`"ARG`"); \
            index__ = uvm_get_array_index_int(str__, wildcard_index__); \
            if(uvm_is_array(str__)  && (index__ != -1)) begin\
              if(wildcard_index__) begin \
                for(index__=0; index__<ARG.size(); ++index__) begin \
                  if(uvm_is_match(str__, {__m_uvm_status_container.scope.get_arg(),$sformatf("[%0d]", index__)})) begin \
                    if (__m_uvm_status_container.print_matches) \
                      uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg(), $sformatf("[%0d]",index__)}, UVM_LOW); \
                    ARG[index__] = uvm_object::__m_uvm_status_container.stringv; \
                    __m_uvm_status_container.status = 1; \
                  end \
                end \
              end \
              else if(uvm_is_match(str__, {__m_uvm_status_container.scope.get_arg(),$sformatf("[%0d]", index__)})) begin \
                if(index__+1 > ARG.size()) begin \
                  int sz = index__; \
                  string tmp__; \
                  `M_UVM_``TYPE``_RESIZE(ARG,tmp__) \
                end \
                if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
                ARG[index__] =  uvm_object::__m_uvm_status_container.stringv; \
                __m_uvm_status_container.status = 1; \
              end \
            end \
          end \
        end \
    endcase \
  end



// MACRO: `uvm_field_array_enum
//
// Implements the data operations for a one-dimensional dynamic array of
// enums.
//
//|  `uvm_field_array_enum(T,ARG,FLAG)
//
// ~T~ is a one-dimensional dynamic array of enums _type_,
// ~ARG~ is an instance of that type, and ~FLAG~ is a bitwise OR of
// one or more flag settings as described in <Field Macros> above.

`define uvm_field_array_enum(T,ARG,FLAG) \
  `M_FIELD_QDA_ENUM(ARRAY,T,ARG,FLAG) 

`define M_FIELD_QDA_ENUM(TYPE,T,ARG,FLAG) \
  begin \
    case (what__) \
      UVM_CHECK_FIELDS: \
        __m_uvm_status_container.do_field_check(`"ARG`", this); \
      UVM_COPY: \
        begin \
          if(!((FLAG)&UVM_NOCOPY)) ARG = local_data__.ARG; \
        end \
      UVM_COMPARE: \
        begin \
          if(!((FLAG)&UVM_NOCOMPARE)) begin \
            if(ARG !== local_data__.ARG) begin \
               if(__m_uvm_status_container.comparer.show_max == 1) begin \
                 __m_uvm_status_container.scope.set_arg(`"ARG`"); \
                 __m_uvm_status_container.comparer.print_msg(""); \
               end \
               else if(__m_uvm_status_container.comparer.show_max) begin \
                 /**/if(ARG.size() != local_data__.ARG.size()) begin \
                 /**/  void'(__m_uvm_status_container.comparer.compare_field(`"ARG``.size`", ARG.size(), local_data__.ARG.size(), 32)); \
                 /**/end \
                 /**/else begin \
                   foreach(ARG[i]) begin \
                     if(ARG[i] !== local_data__.ARG[i]) begin \
                       __m_uvm_status_container.scope.set_arg_element(`"ARG`",i); \
                       $swrite(__m_uvm_status_container.stringv, "lhs = %0s : rhs = %0s", \
                         ARG[i].name(), local_data__.ARG[i].name()); \
                       __m_uvm_status_container.comparer.print_msg(__m_uvm_status_container.stringv); \
                       if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
                     end \
                   end \
                 /**/end \
               end \
               else if ((__m_uvm_status_container.comparer.physical&&((FLAG)&UVM_PHYSICAL)) || \
                        (__m_uvm_status_container.comparer.abstract&&((FLAG)&UVM_ABSTRACT)) || \
                        (!((FLAG)&UVM_PHYSICAL) && !((FLAG)&UVM_ABSTRACT)) ) \
                 __m_uvm_status_container.comparer.result++; \
               if(__m_uvm_status_container.comparer.result && (__m_uvm_status_container.comparer.show_max <= __m_uvm_status_container.comparer.result)) return; \
            end \
          end \
        end \
      UVM_PACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          /**/if(__m_uvm_status_container.packer.use_metadata) __m_uvm_status_container.packer.pack_field_int(ARG.size(), 32); \
          foreach(ARG[i])  \
            __m_uvm_status_container.packer.pack_field_int(int'(ARG[i]), $bits(ARG[i])); \
        end \
      UVM_UNPACK: \
        if(!((FLAG)&UVM_NOPACK)) begin \
          /**/int sz = ARG.size(); \
          /**/if(__m_uvm_status_container.packer.use_metadata) sz = __m_uvm_status_container.packer.unpack_field_int(32); \
          /**/if(sz != ARG.size()) begin \
          /**/  T tmp__; \
          /**/  `M_UVM_``TYPE``_RESIZE(ARG,tmp__) \
          /**/end \
          foreach(ARG[i]) \
            ARG[i] = T'(__m_uvm_status_container.packer.unpack_field_int($bits(ARG[i]))); \
        end \
      UVM_RECORD: \
        /**/`m_uvm_record_qda_enum(ARG,FLAG,ARG.size()) \
      UVM_PRINT: \
        begin \
          if(((FLAG)&UVM_NOPRINT) == 0) begin \
             `uvm_print_qda_enum(ARG, __m_uvm_status_container.printer, array, T) \
          end \
        end \
      UVM_SETINT: \
        begin \
          __m_uvm_status_container.scope.set_arg(`"ARG`"); \
          if(uvm_is_match(str__, __m_uvm_status_container.scope.get())) begin \
            if((FLAG)&UVM_READONLY) begin \
              uvm_report_warning("RDONLY", $sformatf("Readonly argument match %s is ignored",  \
                 __m_uvm_status_container.get_full_scope_arg()), UVM_NONE); \
            end \
            /**/else begin \
            /**/  int sz =  uvm_object::__m_uvm_status_container.bitstream; \
            /**/  if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
            /**/  if(ARG.size() !=  sz) begin \
            /**/    T tmp__; \
            /**/    `M_UVM_``TYPE``_RESIZE(ARG,tmp__) \
            /**/  end \
            /**/  __m_uvm_status_container.status = 1; \
            /**/end \
          end \
          else if(!((FLAG)&UVM_READONLY)) begin \
            bit wildcard_index__; \
            int index__; \
            index__ = uvm_get_array_index_int(str__, wildcard_index__); \
            if(uvm_is_array(str__)  && (index__ != -1)) begin\
              if(wildcard_index__) begin \
                for(index__=0; index__<ARG.size(); ++index__) begin \
                  if(uvm_is_match(str__, {__m_uvm_status_container.scope.get_arg(),$sformatf("[%0d]", index__)})) begin \
                    if (__m_uvm_status_container.print_matches) \
                      uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg(), $sformatf("[%0d]",index__)}, UVM_LOW); \
                    ARG[index__] = T'(uvm_object::__m_uvm_status_container.bitstream); \
                    __m_uvm_status_container.status = 1; \
                  end \
                end \
              end \
              else if(uvm_is_match(str__, {__m_uvm_status_container.scope.get_arg(),$sformatf("[%0d]", index__)})) begin \
                if(index__+1 > ARG.size()) begin \
                  int sz = index__; \
                  T tmp__; \
                  `M_UVM_``TYPE``_RESIZE(ARG,tmp__) \
                end \
                if (__m_uvm_status_container.print_matches) \
                  uvm_report_info("STRMTC", {"set_int()", ": Matched string ", str__, " to field ", __m_uvm_status_container.get_full_scope_arg()}, UVM_LOW); \
                ARG[index__] =  T'(uvm_object::__m_uvm_status_container.bitstream); \
                __m_uvm_status_container.status = 1; \
              end \
            end \
          end \
        end \
    endcase \
  end


//-----------------------------------------------------------------------------
// Group: `uvm_field_queue_* macros
//
// Macros that implement data operations for dynamic queues.
//
//-----------------------------------------------------------------------------

// MACRO: `uvm_field_queue_int
//
// Implements the data operations for a queue of integrals.
//
//|  `uvm_field_queue_int(ARG,FLAG)
//
// ~ARG~ is a one-dimensional queue of integrals,
// and ~FLAG~ is a bitwise OR of one or more flag settings as described in
// <Field Macros> above.

`define uvm_field_queue_int(ARG,FLAG) \
  `M_UVM_FIELD_QDA_INT(QUEUE,ARG,FLAG)

// MACRO: `uvm_field_queue_object
//
// Implements the data operations for a queue of <uvm_object>-based objects.
//
//|  `uvm_field_queue_object(ARG,FLAG)
//
// ~ARG~ is a one-dimensional queue of <uvm_object>-based objects,
// and ~FLAG~ is a bitwise OR of one or more flag settings as described in
// <Field Macros> above.

`define uvm_field_queue_object(ARG,FLAG) \
  `M_UVM_FIELD_QDA_OBJECT(QUEUE,ARG,FLAG)


// MACRO: `uvm_field_queue_string
//
// Implements the data operations for a queue of strings.
//
//|  `uvm_field_queue_string(ARG,FLAG)
//
// ~ARG~ is a one-dimensional queue of strings, and ~FLAG~ is a bitwise
// OR of one or more flag settings as described in <Field Macros> above.

`define uvm_field_queue_string(ARG,FLAG) \
  `M_UVM_FIELD_QDA_STRING(QUEUE,ARG,FLAG)


// MACRO: `uvm_field_queue_enum
//
// Implements the data operations for a one-dimensional queue of enums.
//
//|  `uvm_field_queue_enum(T,ARG,FLAG)
//
// ~T~ is a queue of enums _type_, ~ARG~ is an instance of that type,
// and ~FLAG~ is a bitwise OR of one or more flag settings as described
// in <Field Macros> above.

`define uvm_field_queue_enum(T,ARG,FLAG) \
  `M_FIELD_QDA_ENUM(QUEUE,T,ARG,FLAG)


//-----------------------------------------------------------------------------
// Group: `uvm_field_aa_*_string macros
//
// Macros that implement data operations for associative arrays indexed
// by ~string~.
//
//-----------------------------------------------------------------------------

// MACRO: `uvm_field_aa_int_string
//
// Implements the data operations for an associative array of integrals indexed
// by ~string~.
//
//|  `uvm_field_aa_int_string(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of integrals
// with string key, and ~FLAG~ is a bitwise OR of one or more flag settings as
// described in <Field Macros> above.

`define uvm_field_aa_int_string(ARG, FLAG) \
  begin \
  if(what__==UVM_CHECK_FIELDS) __m_uvm_status_container.do_field_check(`"ARG`", this); \
  `M_UVM_FIELD_DATA_AA_int_string(ARG,FLAG) \
  `M_UVM_FIELD_SET_AA_TYPE(string, INT, ARG, __m_uvm_status_container.bitstream, FLAG)  \
  end


// MACRO: `uvm_field_aa_object_string
//
// Implements the data operations for an associative array of <uvm_object>-based
// objects indexed by ~string~.
//
//|  `uvm_field_aa_object_string(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of objects
// with string key, and ~FLAG~ is a bitwise OR of one or more flag settings as
// described in <Field Macros> above.

`define uvm_field_aa_object_string(ARG, FLAG) \
  begin \
  if(what__==UVM_CHECK_FIELDS) __m_uvm_status_container.do_field_check(`"ARG`", this); \
  `M_UVM_FIELD_DATA_AA_object_string(ARG,FLAG) \
  `M_UVM_FIELD_SET_AA_OBJECT_TYPE(string, ARG, FLAG)  \
  end


// MACRO: `uvm_field_aa_string_string
//
// Implements the data operations for an associative array of strings indexed
// by ~string~.
//
//|  `uvm_field_aa_string_string(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of strings
// with string key, and ~FLAG~ is a bitwise OR of one or more flag settings as
// described in <Field Macros> above.

`define uvm_field_aa_string_string(ARG, FLAG) \
  begin \
  if(what__==UVM_CHECK_FIELDS) __m_uvm_status_container.do_field_check(`"ARG`", this); \
  `M_UVM_FIELD_DATA_AA_string_string(ARG,FLAG) \
  `M_UVM_FIELD_SET_AA_TYPE(string, STR, ARG, __m_uvm_status_container.stringv, FLAG)  \
  end


//-----------------------------------------------------------------------------
// Group: `uvm_field_aa_*_int macros
//
// Macros that implement data operations for associative arrays indexed by an
// integral type.
//
//-----------------------------------------------------------------------------

// MACRO: `uvm_field_aa_object_int
//
// Implements the data operations for an associative array of <uvm_object>-based
// objects indexed by the ~int~ data type.
//
//|  `uvm_field_aa_object_int(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of objects
// with ~int~ key, and ~FLAG~ is a bitwise OR of one or more flag settings as
// described in <Field Macros> above.

`define uvm_field_aa_object_int(ARG, FLAG) \
  begin \
  if(what__==UVM_CHECK_FIELDS) __m_uvm_status_container.do_field_check(`"ARG`", this); \
  `M_UVM_FIELD_DATA_AA_object_int(ARG,FLAG) \
  `M_UVM_FIELD_SET_AA_OBJECT_TYPE(int, ARG, FLAG)  \
  end


// MACRO: `uvm_field_aa_int_int
//
// Implements the data operations for an associative array of integral
// types indexed by the ~int~ data type.
//
//|  `uvm_field_aa_int_int(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of integrals
// with ~int~ key, and ~FLAG~ is a bitwise OR of one or more flag settings as
// described in <Field Macros> above.

`define uvm_field_aa_int_int(ARG, FLAG) \
  `uvm_field_aa_int_key(int, ARG, FLAG) \


// MACRO: `uvm_field_aa_int_int_unsigned
//
// Implements the data operations for an associative array of integral
// types indexed by the ~int unsigned~ data type.
//
//|  `uvm_field_aa_int_int_unsigned(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of integrals
// with ~int unsigned~ key, and ~FLAG~ is a bitwise OR of one or more flag
// settings as described in <Field Macros> above.

`define uvm_field_aa_int_int_unsigned(ARG, FLAG) \
  `uvm_field_aa_int_key(int unsigned, ARG, FLAG)


// MACRO: `uvm_field_aa_int_integer
//
// Implements the data operations for an associative array of integral
// types indexed by the ~integer~ data type.
//
//|  `uvm_field_aa_int_integer(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of integrals
// with ~integer~ key, and ~FLAG~ is a bitwise OR of one or more flag settings
// as described in <Field Macros> above.

`define uvm_field_aa_int_integer(ARG, FLAG) \
  `uvm_field_aa_int_key(integer, ARG, FLAG)


// MACRO: `uvm_field_aa_int_integer_unsigned
//
// Implements the data operations for an associative array of integral
// types indexed by the ~integer unsigned~ data type.
//
//|  `uvm_field_aa_int_integer_unsigned(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of integrals
// with ~integer unsigned~ key, and ~FLAG~ is a bitwise OR of one or more 
// flag settings as described in <Field Macros> above.

`define uvm_field_aa_int_integer_unsigned(ARG, FLAG) \
  `uvm_field_aa_int_key(integer unsigned, ARG, FLAG)


// MACRO: `uvm_field_aa_int_byte
//
// Implements the data operations for an associative array of integral
// types indexed by the ~byte~ data type.
//
//|  `uvm_field_aa_int_byte(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of integrals
// with ~byte~ key, and ~FLAG~ is a bitwise OR of one or more flag settings as
// described in <Field Macros> above.

`define uvm_field_aa_int_byte(ARG, FLAG) \
  `uvm_field_aa_int_key(byte, ARG, FLAG)


// MACRO: `uvm_field_aa_int_byte_unsigned
//
// Implements the data operations for an associative array of integral
// types indexed by the ~byte unsigned~ data type.
//
//|  `uvm_field_aa_int_byte_unsigned(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of integrals
// with ~byte unsigned~ key, and ~FLAG~ is a bitwise OR of one or more flag
// settings as described in <Field Macros> above.

`define uvm_field_aa_int_byte_unsigned(ARG, FLAG) \
  `uvm_field_aa_int_key(byte unsigned, ARG, FLAG)


// MACRO: `uvm_field_aa_int_shortint
//
// Implements the data operations for an associative array of integral
// types indexed by the ~shortint~ data type.
//
//|  `uvm_field_aa_int_shortint(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of integrals
// with ~shortint~ key, and ~FLAG~ is a bitwise OR of one or more flag
// settings as described in <Field Macros> above.

`define uvm_field_aa_int_shortint(ARG, FLAG) \
  `uvm_field_aa_int_key(shortint, ARG, FLAG)


// MACRO: `uvm_field_aa_int_shortint_unsigned
//
// Implements the data operations for an associative array of integral
// types indexed by the ~shortint unsigned~ data type.
//
//|  `uvm_field_aa_int_shortint_unsigned(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of integrals
// with ~shortint unsigned~ key, and ~FLAG~ is a bitwise OR of one or more
// flag settings as described in <Field Macros> above.

`define uvm_field_aa_int_shortint_unsigned(ARG, FLAG) \
  `uvm_field_aa_int_key(shortint unsigned, ARG, FLAG)


// MACRO: `uvm_field_aa_int_longint
//
// Implements the data operations for an associative array of integral
// types indexed by the ~longint~ data type.
//
//|  `uvm_field_aa_int_longint(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of integrals
// with ~longint~ key, and ~FLAG~ is a bitwise OR of one or more flag settings
// as described in <Field Macros> above.

`define uvm_field_aa_int_longint(ARG, FLAG) \
  `uvm_field_aa_int_key(longint, ARG, FLAG)


// MACRO: `uvm_field_aa_int_longint_unsigned
//
// Implements the data operations for an associative array of integral
// types indexed by the ~longint unsigned~ data type.
//
//|  `uvm_field_aa_int_longint_unsigned(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of integrals
// with ~longint unsigned~ key, and ~FLAG~ is a bitwise OR of one or more
// flag settings as described in <Field Macros> above.

`define uvm_field_aa_int_longint_unsigned(ARG, FLAG) \
  `uvm_field_aa_int_key(longint unsigned, ARG, FLAG)


// MACRO: `uvm_field_aa_int_key
//
// Implements the data operations for an associative array of integral
// types indexed by any integral key data type. 
//
//|  `uvm_field_aa_int_key(long unsigned,ARG,FLAG)
//
// ~KEY~ is the data type of the integral key, ~ARG~ is the name of a property 
// that is an associative array of integrals, and ~FLAG~ is a bitwise OR of one 
// or more flag settings as described in <Field Macros> above.

`define uvm_field_aa_int_key(KEY, ARG, FLAG) \
  begin \
  if(what__==UVM_CHECK_FIELDS) __m_uvm_status_container.do_field_check(`"ARG`", this); \
  `M_UVM_FIELD_DATA_AA_int_key(KEY,ARG,FLAG) \
  `M_UVM_FIELD_SET_AA_INT_TYPE(KEY, INT, ARG, __m_uvm_status_container.bitstream, FLAG)  \
  end


// MACRO: `uvm_field_aa_int_enumkey
//
// Implements the data operations for an associative array of integral
// types indexed by any enumeration key data type. 
//
//|  `uvm_field_aa_int_longint_unsigned(ARG,FLAG)
//
// ~ARG~ is the name of a property that is an associative array of integrals
// with ~longint unsigned~ key, and ~FLAG~ is a bitwise OR of one or more
// flag settings as described in <Field Macros> above.

`define uvm_field_aa_int_enumkey(KEY, ARG, FLAG) \
  begin \
  if(what__==UVM_CHECK_FIELDS) __m_uvm_status_container.do_field_check(`"ARG`", this); \
  `M_UVM_FIELD_DATA_AA_enum_key(KEY,ARG,FLAG) \
  `M_UVM_FIELD_SET_AA_INT_ENUMTYPE(KEY, INT, ARG, __m_uvm_status_container.bitstream, FLAG)  \
  end

//-----------------------------------------------------------------------------
//
// MACROS- recording
//
//-----------------------------------------------------------------------------

// m_uvm_record_int
// ----------------

// Purpose: provide print functionality for a specific integral field. This
// macro is available for user access. If used externally, a record_options
// object must be avaialble and must have the name opt.
// 
// Postcondition: ~ARG~ is printed using the format set by the FLAGS.

`define m_uvm_record_int(ARG,FLAG) \
  if(!((FLAG)&UVM_NORECORD)) begin \
    __m_uvm_status_container.recorder.record_field(`"ARG`", ARG,  $bits(ARG), uvm_radix_enum'((FLAG)&(UVM_RADIX))); \
  end


// m_uvm_record_string
// -------------------

// Purpose: provide record functionality for a specific string field. This
// macro is available for user access. If used externally, a record_options
// object must be avaialble and must have the name recorder.
//  
// Postcondition: ~ARG~ is recorded in string format.
      

`define m_uvm_record_string(ARG,STR,FLAG) \
  if(!((FLAG)&UVM_NORECORD)) begin \
    __m_uvm_status_container.recorder.record_string(`"ARG`", STR); \
  end


// m_uvm_record_object
// -------------------

// Purpose: provide record functionality for a specific <uvm_object> field. This
// macro is available for user access. If used externally, a record_options
// object must be avaialble and must have the name recorder.
//
// Postcondition: ~ARG~ is recorded. The record is done recursively where the
// depth to record is set in the recorder object.


`define m_uvm_record_object(ARG,FLAG) \
  if(!((FLAG)&UVM_NORECORD)) begin \
    __m_uvm_status_container.recorder.record_object(`"ARG`", ARG); \
  end


// m_uvm_record_qda_int
// --------------------

`define m_uvm_record_qda_int(ARG, FLAG, SZ) \
  begin \
    if(!((FLAG)&UVM_NORECORD)) begin \
      int sz__ = SZ; \
      if(sz__ == 0) begin \
        __m_uvm_status_container.recorder.record_field(`"ARG`", 0, 32, UVM_DEC); \
      end \
      else if(sz__ < 10) begin \
        foreach(ARG[i]) begin \
           __m_uvm_status_container.scope.set_arg_element(`"ARG`",i); \
           __m_uvm_status_container.recorder.record_field(__m_uvm_status_container.scope.get(), ARG[i], $bits(ARG[i]), uvm_radix_enum'((FLAG)&UVM_RADIX)); \
        end \
      end \
      else begin \
        for(int i=0; i<5; ++i) begin \
           __m_uvm_status_container.scope.set_arg_element(`"ARG`", i); \
           __m_uvm_status_container.recorder.record_field(__m_uvm_status_container.scope.get(), ARG[i], $bits(ARG[i]), uvm_radix_enum'((FLAG)&UVM_RADIX)); \
        end \
        for(int i=sz__-5; i<sz__; ++i) begin \
           __m_uvm_status_container.scope.set_arg_element(`"ARG`", i); \
           __m_uvm_status_container.recorder.record_field(__m_uvm_status_container.scope.get(), ARG[i], $bits(ARG[i]), uvm_radix_enum'((FLAG)&UVM_RADIX)); \
        end \
      end \
    end \
  end


// m_uvm_record_qda_enum
// ---------------------

`define m_uvm_record_qda_enum(ARG, FLAG, SZ) \
  begin \
    if(!((FLAG)&UVM_NORECORD) && (__m_uvm_status_container.recorder.tr_handle != 0)) begin \
      int sz__ = SZ; \
      if(sz__ == 0) begin \
        __m_uvm_status_container.recorder.record_field(`"ARG``.size`", 0, 32, UVM_DEC); \
      end \
      else if(sz__ < 10) begin \
        foreach(ARG[i]) begin \
           __m_uvm_status_container.scope.set_arg_element(`"ARG`",i); \
           __m_uvm_status_container.recorder.record_string(__m_uvm_status_container.scope.get(), ARG[i].name()); \
        end \
      end \
      else begin \
        for(int i=0; i<5; ++i) begin \
           __m_uvm_status_container.scope.set_arg_element(`"ARG`", i); \
           __m_uvm_status_container.recorder.record_string(__m_uvm_status_container.scope.get(), ARG[i].name()); \
        end \
        for(int i=sz__-5; i<sz__; ++i) begin \
           __m_uvm_status_container.scope.set_arg_element(`"ARG`", i); \
           __m_uvm_status_container.recorder.record_string(__m_uvm_status_container.scope.get(), ARG[i].name()); \
        end \
      end \
    end \
  end


// m_uvm_record_qda_object
// -----------------------

`define m_uvm_record_qda_object(ARG, FLAG, SZ) \
  begin \
    if(!((FLAG)&UVM_NORECORD)) begin \
      int sz__ = SZ; \
      string s; \
      if(sz__ == 0 ) begin \
        __m_uvm_status_container.recorder.record_field(`"ARG``.size`", 0, 32, UVM_DEC); \
      end \
      if(sz__ < 10) begin \
        foreach(ARG[i]) begin \
           $swrite(s,`"ARG[%0d]`", i); \
           __m_uvm_status_container.recorder.record_object(s, ARG[i]); \
        end \
      end \
      else begin \
        for(int i=0; i<5; ++i) begin \
           $swrite(s,`"ARG[%0d]`", i); \
           __m_uvm_status_container.recorder.record_object(s, ARG[i]); \
        end \
        for(int i=sz__-5; i<sz__; ++i) begin \
           $swrite(s,`"ARG[%0d]`", i); \
           __m_uvm_status_container.recorder.record_object(s, ARG[i]); \
        end \
      end \
    end \
  end


// m_uvm_record_qda_string
// -----------------------

`define m_uvm_record_qda_string(ARG, FLAG, SZ) \
  begin \
    int sz__ = SZ; \
    if(!((FLAG)&UVM_NORECORD)) begin \
      if(sz__ == 0) begin \
        __m_uvm_status_container.recorder.record_field(`"ARG``.size`", 0, 32, UVM_DEC); \
      end \
      else if(sz__ < 10) begin \
        foreach(ARG[i]) begin \
           __m_uvm_status_container.scope.set_arg_element(`"ARG`",i); \
           __m_uvm_status_container.recorder.record_string(__m_uvm_status_container.scope.get(), ARG[i]); \
        end \
      end \
      else begin \
        for(int i=0; i<5; ++i) begin \
           __m_uvm_status_container.scope.set_arg_element(`"ARG`", i); \
           __m_uvm_status_container.recorder.record_string(__m_uvm_status_container.scope.get(), ARG[i]); \
        end \
        for(int i=sz__-5; i<sz__; ++i) begin \
           __m_uvm_status_container.scope.set_arg_element(`"ARG`", i); \
           __m_uvm_status_container.recorder.record_string(__m_uvm_status_container.scope.get(), ARG[i]); \
        end \
      end \
    end \
  end


// M_UVM_FIELD_DATA_AA_generic
// -------------------------

`define M_UVM_FIELD_DATA_AA_generic(TYPE, KEY, ARG, FLAG) \
  begin \
    begin \
      case (what__) \
        UVM_COMPARE: \
           begin \
            if(!((FLAG)&UVM_NOCOMPARE) && (tmp_data__ != null) ) \
            begin \
              $cast(local_data__, tmp_data__); \
              if(ARG.num() != local_data__.ARG.num()) begin \
                 int s1__, s2__; \
                 __m_uvm_status_container.stringv = ""; \
                 s1__ = ARG.num(); s2__ = local_data__.ARG.num(); \
                 $swrite(__m_uvm_status_container.stringv, "lhs size = %0d : rhs size = %0d", \
                    s1__, s2__);\
                 __m_uvm_status_container.comparer.print_msg(__m_uvm_status_container.stringv); \
              end \
              string_aa_key = ""; \
              while(ARG.next(string_aa_key)) begin \
                string s; \
                __m_uvm_status_container.scope.set_arg({"[",string_aa_key,"]"}); \
                s = {`"ARG[`",string_aa_key,"]"}; \
                if($bits(ARG[string_aa_key]) <= 64) \
                  void'(__m_uvm_status_container.comparer.compare_field_int(s, ARG[string_aa_key], local_data__.ARG[string_aa_key], $bits(ARG[string_aa_key]), uvm_radix_enum'((FLAG)&UVM_RADIX))); \
                else \
                  void'(__m_uvm_status_container.comparer.compare_field(s, ARG[string_aa_key], local_data__.ARG[string_aa_key], $bits(ARG[string_aa_key]), uvm_radix_enum'((FLAG)&UVM_RADIX))); \
                __m_uvm_status_container.scope.unset_arg(string_aa_key); \
              end \
            end \
           end \
        UVM_COPY: \
          begin \
            if(!((FLAG)&UVM_NOCOPY) && (tmp_data__ != null) ) \
            begin \
              $cast(local_data__, tmp_data__); \
              ARG.delete(); \
              string_aa_key = ""; \
              while(local_data__.ARG.next(string_aa_key)) \
                ARG[string_aa_key] = local_data__.ARG[string_aa_key]; \
            end \
          end \
        UVM_PRINT: \
          if(!((FLAG)&UVM_NOPRINT)) begin \
            `uvm_print_aa_``KEY``_``TYPE``3(ARG, uvm_radix_enum'((FLAG)&(UVM_RADIX)), \
                                            __m_uvm_status_container.printer) \
          end \
      endcase \
    end \
  end


// M_UVM_FIELD_DATA_AA_int_string
// ----------------------------

`define M_UVM_FIELD_DATA_AA_int_string(ARG, FLAG) \
  `M_UVM_FIELD_DATA_AA_generic(int, string, ARG, FLAG)


// M_UVM_FIELD_DATA_AA_int_int
// ----------------------------

`define M_UVM_FIELD_DATA_AA_int_key(KEY, ARG, FLAG) \
  begin \
    begin \
      KEY aa_key; \
      case (what__) \
        UVM_COMPARE: \
           begin \
            if(!((FLAG)&UVM_NOCOMPARE) && (tmp_data__ != null) ) \
            begin \
              $cast(local_data__, tmp_data__); \
              if(ARG.num() != local_data__.ARG.num()) begin \
                 int s1__, s2__; \
                 __m_uvm_status_container.stringv = ""; \
                 s1__ = ARG.num(); s2__ = local_data__.ARG.num(); \
                 $swrite(__m_uvm_status_container.stringv, "lhs size = %0d : rhs size = %0d", \
                    s1__, s2__);\
                 __m_uvm_status_container.comparer.print_msg(__m_uvm_status_container.stringv); \
              end \
              if(ARG.first(aa_key)) \
                do begin \
                  string s; \
                  $swrite(string_aa_key, "%0d", aa_key); \
                  __m_uvm_status_container.scope.set_arg({"[",string_aa_key,"]"}); \
                  s = {`"ARG[`",string_aa_key,"]"}; \
                  if($bits(ARG[aa_key]) <= 64) \
                    void'(__m_uvm_status_container.comparer.compare_field_int(s, ARG[aa_key], local_data__.ARG[aa_key], $bits(ARG[aa_key]), uvm_radix_enum'((FLAG)&UVM_RADIX))); \
                  else \
                    void'(__m_uvm_status_container.comparer.compare_field(s, ARG[aa_key], local_data__.ARG[aa_key], $bits(ARG[aa_key]), uvm_radix_enum'((FLAG)&UVM_RADIX))); \
                  __m_uvm_status_container.scope.unset_arg(string_aa_key); \
                end while(ARG.next(aa_key)); \
            end \
           end \
        UVM_COPY: \
          begin \
            if(!((FLAG)&UVM_NOCOPY) && (tmp_data__ != null) ) \
            begin \
              $cast(local_data__, tmp_data__); \
              ARG.delete(); \
              if(local_data__.ARG.first(aa_key)) \
                do begin \
                  ARG[aa_key] = local_data__.ARG[aa_key]; \
                end while(local_data__.ARG.next(aa_key)); \
            end \
          end \
        UVM_PRINT: \
          if(!((FLAG)&UVM_NOPRINT)) begin \
             `uvm_print_aa_int_key4(KEY,ARG, uvm_radix_enum'((FLAG)&(UVM_RADIX)), \
                                    __m_uvm_status_container.printer) \
          end \
      endcase \
    end \
  end


// M_UVM_FIELD_DATA_AA_enum_key
// ----------------------------

`define M_UVM_FIELD_DATA_AA_enum_key(KEY, ARG, FLAG) \
  begin \
    begin \
      KEY aa_key; \
      case (what__) \
        UVM_COMPARE: \
           begin \
            if(!((FLAG)&UVM_NOCOMPARE) && (tmp_data__ != null) ) \
            begin \
              $cast(local_data__, tmp_data__); \
              if(ARG.num() != local_data__.ARG.num()) begin \
                 int s1__, s2__; \
                 __m_uvm_status_container.stringv = ""; \
                 s1__ = ARG.num(); s2__ = local_data__.ARG.num(); \
                 $swrite(__m_uvm_status_container.stringv, "lhs size = %0d : rhs size = %0d", \
                    s1__, s2__);\
                 __m_uvm_status_container.comparer.print_msg(__m_uvm_status_container.stringv); \
              end \
              if(ARG.first(aa_key)) \
                do begin \
                  void'(__m_uvm_status_container.comparer.compare_field_int({`"ARG[`",aa_key.name(),"]"}, \
                    ARG[aa_key], local_data__.ARG[aa_key], $bits(ARG[aa_key]), \
                    uvm_radix_enum'((FLAG)&UVM_RADIX) )); \
                end while(ARG.next(aa_key)); \
            end \
           end \
        UVM_COPY: \
          begin \
            if(!((FLAG)&UVM_NOCOPY) && (tmp_data__ != null) ) \
            begin \
              $cast(local_data__, tmp_data__); \
              ARG.delete(); \
              if(local_data__.ARG.first(aa_key)) \
                do begin \
                  ARG[aa_key] = local_data__.ARG[aa_key]; \
                end while(local_data__.ARG.next(aa_key)); \
            end \
          end \
        UVM_PRINT: \
          if(!((FLAG)&UVM_NOPRINT)) begin \
            uvm_printer p__ = __m_uvm_status_container.printer; \
            p__.print_array_header (`"ARG`", ARG.num(),`"aa_``KEY`"); \
            if((p__.knobs.depth == -1) || (__m_uvm_status_container.printer.m_scope.depth() < p__.knobs.depth+1)) \
            begin \
              if(ARG.first(aa_key)) \
                do begin \
                  __m_uvm_status_container.printer.print_int( \
                    {"[",aa_key.name(),"]"}, ARG[aa_key], $bits(ARG[aa_key]), \
                    uvm_radix_enum'((FLAG)&UVM_RADIX), "[" ); \
                end while(ARG.next(aa_key)); \
            end \
            p__.print_array_footer(ARG.num()); \
            //p__.print_footer(); \
          end \
      endcase \
    end \
  end 


// M_UVM_FIELD_DATA_AA_object_string
// -------------------------------

`define M_UVM_FIELD_DATA_AA_object_string(ARG, FLAG) \
  begin \
    begin \
      case (what__) \
        UVM_COMPARE: \
           begin \
            if(!((FLAG)&UVM_NOCOMPARE) && (tmp_data__ != null) ) \
            begin \
              $cast(local_data__, tmp_data__); \
              if(ARG.num() != local_data__.ARG.num()) begin \
                 int s1__, s2__; \
                 __m_uvm_status_container.stringv = ""; \
                 s1__ = ARG.num(); s2__ = local_data__.ARG.num(); \
                 $swrite(__m_uvm_status_container.stringv, "lhs size = %0d : rhs size = %0d", \
                          s1__, s2__);\
                 __m_uvm_status_container.comparer.print_msg(__m_uvm_status_container.stringv); \
              end \
              string_aa_key = ""; \
              while(ARG.next(string_aa_key)) begin \
                uvm_object lhs; \
                uvm_object rhs; \
                lhs = ARG[string_aa_key]; \
                rhs = local_data__.ARG[string_aa_key]; \
                __m_uvm_status_container.scope.down({"[",string_aa_key,"]"}); \
                //if the object are the same then don't need to do a deep compare \
                if(rhs != lhs) begin \
                  bit refcmp; \
                  refcmp = ((FLAG)& UVM_SHALLOW) && !(__m_uvm_status_container.comparer.policy == UVM_DEEP); \
                  //do a deep compare here  \
                  if(!refcmp && !(__m_uvm_status_container.comparer.policy == UVM_REFERENCE)) begin \
                    if(((rhs == null) && (lhs != null)) || ((lhs==null) && (rhs!=null))) begin \
                      __m_uvm_status_container.comparer.print_msg_object(lhs, rhs); \
                    end \
                    else begin \
                      if (lhs != null)  \
                        void'(lhs.compare(rhs, __m_uvm_status_container.comparer)); \
                    end \
                  end \
                  else begin //reference compare \
                    __m_uvm_status_container.comparer.print_msg_object(lhs, rhs); \
                  end \
                end \
                __m_uvm_status_container.scope.up_element(); \
              end \
            end \
          end \
        UVM_COPY: \
          begin \
           if(!((FLAG)&UVM_NOCOPY) && (tmp_data__ != null) ) \
           begin \
            $cast(local_data__, tmp_data__); \
            ARG.delete(); \
            if(local_data__.ARG.first(string_aa_key)) \
             do \
               if((FLAG)&UVM_REFERENCE) \
                ARG[string_aa_key] = local_data__.ARG[string_aa_key]; \
             /*else if((FLAG)&UVM_SHALLOW)*/ \
             /* ARG[string_aa_key] = new local_data__.ARG[string_aa_key];*/ \
               else begin\
                $cast(ARG[string_aa_key],local_data__.ARG[string_aa_key].clone());\
                ARG[string_aa_key].set_name({`"ARG`","[",string_aa_key, "]"});\
               end \
             while(local_data__.ARG.next(string_aa_key)); \
           end \
          end \
        UVM_PRINT: \
          if(!((FLAG)&UVM_NOPRINT)) begin \
            `uvm_print_aa_string_object3(ARG, __m_uvm_status_container.printer,FLAG) \
          end \
      endcase \
    end \
  end


// M_UVM_FIELD_DATA_AA_object_int
// -------------------------------

`define M_UVM_FIELD_DATA_AA_object_int(ARG, FLAG) \
  begin \
    int key__; \
    begin \
      case (what__) \
        UVM_COMPARE: \
           begin \
            if(!((FLAG)&UVM_NOCOMPARE) && (tmp_data__ != null) ) \
            begin \
              $cast(local_data__, tmp_data__); \
              if(ARG.num() != local_data__.ARG.num()) begin \
                 int s1__, s2__; \
                 __m_uvm_status_container.stringv = ""; \
                 s1__ = ARG.num(); s2__ = local_data__.ARG.num(); \
                 $swrite(__m_uvm_status_container.stringv, "lhs size = %0d : rhs size = %0d", \
                          s1__, s2__);\
                 __m_uvm_status_container.comparer.print_msg(__m_uvm_status_container.stringv); \
              end \
              if(ARG.first(key__)) begin \
                do begin \
                  uvm_object lhs; \
                  uvm_object rhs; \
                  lhs = ARG[key__]; \
                  rhs = local_data__.ARG[key__]; \
                  __m_uvm_status_container.scope.down_element(key__); \
                  if(rhs != lhs) begin \
                    bit refcmp; \
                    refcmp = ((FLAG)& UVM_SHALLOW) && !(__m_uvm_status_container.comparer.policy == UVM_DEEP); \
                    //do a deep compare here  \
                    if(!refcmp && !(__m_uvm_status_container.comparer.policy == UVM_REFERENCE)) begin \
                      if(((rhs == null) && (lhs != null)) || ((lhs==null) && (rhs!=null))) begin \
                        __m_uvm_status_container.comparer.print_msg_object(lhs, rhs); \
                      end \
                      else begin \
                        if (lhs != null)  \
                          void'(lhs.compare(rhs, __m_uvm_status_container.comparer)); \
                      end \
                    end \
                    else begin //reference compare \
                      __m_uvm_status_container.comparer.print_msg_object(lhs, rhs); \
                    end \
                  end \
                  __m_uvm_status_container.scope.up_element(); \
                end while(ARG.next(key__)); \
              end \
            end \
          end \
        UVM_COPY: \
          begin \
           if(!((FLAG)&UVM_NOCOPY) && (tmp_data__ != null) ) \
           begin \
            $cast(local_data__, tmp_data__); \
            ARG.delete(); \
            if(local_data__.ARG.first(key__)) \
             do begin \
               if((FLAG)&UVM_REFERENCE) \
                ARG[key__] = local_data__.ARG[key__]; \
             /*else if((FLAG)&UVM_SHALLOW)*/ \
             /* ARG[key__] = new local_data__.ARG[key__];*/ \
               else begin\
                 uvm_object tmp_obj; \
                 tmp_obj = local_data__.ARG[key__].clone(); \
                 if(tmp_obj != null) \
                   $cast(ARG[key__], tmp_obj); \
                 else \
                   ARG[key__]=null; \
               end \
             end while(local_data__.ARG.next(key__)); \
           end \
         end \
        UVM_PRINT: \
          if(!((FLAG)&UVM_NOPRINT)) begin \
             `uvm_print_aa_int_object3(ARG, __m_uvm_status_container.printer,FLAG) \
          end \
      endcase \
    end \
  end


// M_UVM_FIELD_DATA_AA_string_string
// -------------------------------

`define M_UVM_FIELD_DATA_AA_string_string(ARG, FLAG) \
  begin \
    begin \
      case (what__) \
        UVM_COPY: \
          begin \
            if(!((FLAG)&UVM_NOCOPY) && (local_data__ !=null)) \
              ARG = local_data__.ARG ; \
          end \
        UVM_PRINT: \
          if(!((FLAG)&UVM_NOPRINT)) begin \
            `uvm_print_aa_string_string2(ARG, __m_uvm_status_container.printer) \
          end \
        UVM_COMPARE: \
           begin \
            if(!((FLAG)&UVM_NOCOMPARE) && (tmp_data__ != null) ) \
            begin \
              $cast(local_data__, tmp_data__); \
              if(ARG.num() != local_data__.ARG.num()) begin \
                 int s1__, s2__; \
                 __m_uvm_status_container.stringv = ""; \
                 s1__ = ARG.num(); s2__ = local_data__.ARG.num(); \
                 $swrite(__m_uvm_status_container.stringv, "lhs size = %0d : rhs size = %0d", \
                    s1__, s2__);\
                 __m_uvm_status_container.comparer.print_msg(__m_uvm_status_container.stringv); \
              end \
              string_aa_key = ""; \
              while(ARG.next(string_aa_key)) begin \
                string s__ = ARG[string_aa_key]; \
                __m_uvm_status_container.scope.set_arg({"[",string_aa_key,"]"}); \
                if(ARG[string_aa_key] != local_data__.ARG[string_aa_key]) begin \
                   __m_uvm_status_container.stringv = { "lhs = \"", s__, "\" : rhs = \"", local_data__.ARG[string_aa_key], "\""}; \
                   __m_uvm_status_container.comparer.print_msg(__m_uvm_status_container.stringv); \
                end \
                __m_uvm_status_container.scope.unset_arg(string_aa_key); \
              end \
            end \
           end \
      endcase \
    end \
  end


// M_UVM_FIELD_SET_AA_TYPE
// -----------------------

`define M_UVM_FIELD_SET_AA_TYPE(INDEX_TYPE, ARRAY_TYPE, ARRAY, RHS, FLAG) \
  if((what__ >= UVM_START_FUNCS && what__ <= UVM_END_FUNCS) && (((FLAG)&UVM_READONLY) == 0)) begin \
    bit wildcard_index__; \
    INDEX_TYPE index__; \
    index__ = uvm_get_array_index_``INDEX_TYPE(str__, wildcard_index__); \
    if(what__==UVM_SET``ARRAY_TYPE) \
    begin \
      __m_uvm_status_container.scope.down(`"ARRAY`"); \
      if(uvm_is_array(str__) ) begin\
        if(wildcard_index__) begin \
          if(ARRAY.first(index__)) \
          do begin \
            if(uvm_is_match(str__, {__m_uvm_status_container.scope.get(),$sformatf("[%0d]", index__)}) ||  \
               uvm_is_match(str__, {__m_uvm_status_container.scope.get(),$sformatf("[%0s]", index__)})) begin \
              ARRAY[index__] = RHS; \
              __m_uvm_status_container.status = 1; \
            end \
          end while(ARRAY.next(index__));\
        end \
        else if(uvm_is_match(str__, {__m_uvm_status_container.scope.get(),$sformatf("[%0d]", index__)})) begin \
          ARRAY[index__] = RHS; \
          __m_uvm_status_container.status = 1; \
        end \
        else if(uvm_is_match(str__, {__m_uvm_status_container.scope.get(),$sformatf("[%0s]", index__)})) begin \
          ARRAY[index__] = RHS; \
          __m_uvm_status_container.status = 1; \
        end \
      end \
      __m_uvm_status_container.scope.up(); \
    end \
 end


// M_UVM_FIELD_SET_AA_OBJECT_TYPE
// ------------------------------

`define M_UVM_FIELD_SET_AA_OBJECT_TYPE(INDEX_TYPE, ARRAY, FLAG) \
  if((what__ >= UVM_START_FUNCS && what__ <= UVM_END_FUNCS) && (((FLAG)&UVM_READONLY) == 0)) begin \
    bit wildcard_index__; \
    INDEX_TYPE index__; \
    index__ = uvm_get_array_index_``INDEX_TYPE(str__, wildcard_index__); \
    if(what__==UVM_SETOBJ) \
    begin \
      __m_uvm_status_container.scope.down(`"ARRAY`"); \
      if(uvm_is_array(str__) ) begin\
        if(wildcard_index__) begin \
          if(ARRAY.first(index__)) \
          do begin \
            if(uvm_is_match(str__, {__m_uvm_status_container.scope.get(),$sformatf("[%0d]", index__)}) || \
               uvm_is_match(str__, {__m_uvm_status_container.scope.get(),$sformatf("[%0s]", index__)})) begin \
              if (__m_uvm_status_container.object != null) \
                $cast(ARRAY[index__], __m_uvm_status_container.object); \
              __m_uvm_status_container.status = 1; \
            end \
          end while(ARRAY.next(index__));\
        end \
        else if(uvm_is_match(str__, {__m_uvm_status_container.scope.get(),$sformatf("[%0d]", index__)})) begin \
          if (__m_uvm_status_container.object != null) \
            $cast(ARRAY[index__], __m_uvm_status_container.object); \
          __m_uvm_status_container.status = 1; \
        end \
        else if(uvm_is_match(str__, {__m_uvm_status_container.scope.get(),$sformatf("[%0s]", index__)})) begin \
          if (__m_uvm_status_container.object != null) \
            $cast(ARRAY[index__], __m_uvm_status_container.object); \
          __m_uvm_status_container.status = 1; \
        end \
      end \
      __m_uvm_status_container.scope.up(); \
    end \
 end


// M_UVM_FIELD_SET_AA_INT_TYPE
// ---------------------------

`define M_UVM_FIELD_SET_AA_INT_TYPE(INDEX_TYPE, ARRAY_TYPE, ARRAY, RHS, FLAG) \
  if((what__ >= UVM_START_FUNCS && what__ <= UVM_END_FUNCS) && (((FLAG)&UVM_READONLY) == 0)) begin \
    bit wildcard_index__; \
    INDEX_TYPE index__; \
    string idx__; \
    index__ = uvm_get_array_index_int(str__, wildcard_index__); \
    if(what__==UVM_SET``ARRAY_TYPE) \
    begin \
      __m_uvm_status_container.scope.down(`"ARRAY`"); \
      if(uvm_is_array(str__) ) begin\
        if(wildcard_index__) begin \
          if(ARRAY.first(index__)) \
          do begin \
            $swrite(idx__, __m_uvm_status_container.scope.get(), "[", index__, "]"); \
            if(uvm_is_match(str__, idx__)) begin \
              ARRAY[index__] = RHS; \
              __m_uvm_status_container.status = 1; \
            end \
          end while(ARRAY.next(index__));\
        end \
        else if(uvm_is_match(str__, {__m_uvm_status_container.scope.get(),$sformatf("[%0d]", index__)})) begin \
          ARRAY[index__] = RHS; \
          __m_uvm_status_container.status = 1; \
        end  \
      end \
      __m_uvm_status_container.scope.up(); \
    end \
 end


// M_UVM_FIELD_SET_AA_INT_ENUMTYPE
// -------------------------------

`define M_UVM_FIELD_SET_AA_INT_ENUMTYPE(INDEX_TYPE, ARRAY_TYPE, ARRAY, RHS, FLAG) \
  if((what__ >= UVM_START_FUNCS && what__ <= UVM_END_FUNCS) && (((FLAG)&UVM_READONLY) == 0)) begin \
    bit wildcard_index__; \
    INDEX_TYPE index__; \
    string idx__; \
    index__ = INDEX_TYPE'(uvm_get_array_index_int(str__, wildcard_index__)); \
    if(what__==UVM_SET``ARRAY_TYPE) \
    begin \
      __m_uvm_status_container.scope.down(`"ARRAY`"); \
      if(uvm_is_array(str__) ) begin\
        if(wildcard_index__) begin \
          if(ARRAY.first(index__)) \
          do begin \
            $swrite(idx__, __m_uvm_status_container.scope.get(), "[", index__, "]"); \
            if(uvm_is_match(str__, idx__)) begin \
              ARRAY[index__] = RHS; \
              __m_uvm_status_container.status = 1; \
            end \
          end while(ARRAY.next(index__));\
        end \
        else if(uvm_is_match(str__, {__m_uvm_status_container.scope.get(),$sformatf("[%0d]", index__)})) begin \
          ARRAY[index__] = RHS; \
          __m_uvm_status_container.status = 1; \
        end  \
      end \
      __m_uvm_status_container.scope.up(); \
    end \
 end

`endif //!UVM_EMPTY_MACROS




//------------------------------------------------------------------------------
// Group: Recording Macros
//
// The recording macros assist users who implement the <uvm_object::do_record>
// method. They help ensure that the fields are recorded using a vendor-
// independent API. Unlike the <uvm_recorder> policy, fields recorded using
// the <`uvm_record_field> macro do not lose type information--they are passed
// directly to the vendor-specific API. This results in more efficient recording
// and no artificial limit on bit-widths. See your simulator vendor's 
// documentation for more information on its transaction recording capabilities.
//------------------------------------------------------------------------------


// Macro: `uvm_record_attribute
//
// Vendor-independent macro to hide vendor-specific interface for
// recording attributes (fields) to a transaction database.

`ifndef uvm_record_attribute
  `ifdef QUESTA
    `define uvm_record_attribute(TR_HANDLE,NAME,VALUE) \
      $add_attribute(TR_HANDLE,VALUE,NAME);
  `elsif VCS
    `define uvm_record_attribute(TR_HANDLE,NAME,VALUE) \
      // need VCS call here
  `elsif INCA
    `define uvm_record_attribute(TR_HANDLE,NAME,VALUE) \
      // need IUS call here
  `else
    `define uvm_record_attribute(TR_HANDLE,NAME,VALUE) \
      // empty definition
  `endif
`endif

  

// Macro: `uvm_record_field
//
// Macro for recording name-value pairs into a transaction recording database.
// Requires a valid transaction handle, as provided by the
// <uvm_transaction::begin_tr> and <uvm_component::begin_tr> methods. 

`define uvm_record_field(NAME,VALUE) \
   if (recorder != null && recorder.tr_handle != 0) begin \
     if (recorder.get_type_name() != "uvm_recorder") begin \
       `uvm_record_attribute(recorder.tr_handle,NAME,VALUE) \
     end \
     else \
       recorder.m_set_attribute(recorder.tr_handle,NAME,$sformatf("%p",VALUE)); \
   end



// Use the following if the simulator's recording API can not
// distinguish types.

`define uvm_record_int(NAME,VALUE,SIZE,RADIX) \
  recorder.m_set_attribute(recorder.tr_handle,NAME, \
     $sformatf({"%0",uvm_radix_to_string(RADIX)},VALUE)); \

`define uvm_record_string(NAME,VALUE) \
  recorder.m_set_attribute(recorder.tr_handle,NAME,VALUE);

`define uvm_record_time(NAME,VALUE) \
  recorder.m_set_attribute(recorder.tr_handle,NAME, \
     $sformatf("%0u",VALUE&((1<<64)-1))); \

`define uvm_record_real(NAME,VALUE) \
  begin \
  bit[63:0] ival = $realtobits(VALUE); \
  recorder.m_set_attribute(recorder.tr_handle,NAME,ival); \
  end

  
//------------------------------------------------------------------------------
// Group: Packing Macros
//
// The packing macros assist users who implement the <uvm_object::do_pack>
// method. They help ensure that the pack operation is the exact inverse of the
// unpack operation. See also <Unpacking Macros>.
//
//| virtual function void do_pack(uvm_packer packer);
//|   `uvm_pack_int(cmd)
//|   `uvm_pack_int(addr)
//|   `uvm_pack_array(data)
//| endfunction
//
// The 'N' versions of these macros take a explicit size argument.
//------------------------------------------------------------------------------

//--------------------------------
// Group: Packing - With Size Info
//--------------------------------

// Macro: `uvm_pack_intN
//
// Pack an integral variable.
//
//| `uvm_pack_intN(VAR,SIZE)
//
`define uvm_pack_intN(VAR,SIZE) \
   begin \
   if (packer.big_endian) begin \
     longint tmp__ = VAR; \
     for (int i=0; i<SIZE; i++) \
       packer.m_bits[packer.count + i] = tmp__[SIZE-1-i]; \
   end \
   else begin \
     packer.m_bits[packer.count +: SIZE] = VAR; \
   end \
   packer.count += SIZE; \
   end

// Macro: `uvm_pack_enumN
//
// Pack an integral variable.
//
//| `uvm_pack_enumN(VAR,SIZE)
//
`define uvm_pack_enumN(VAR,SIZE) \
   `uvm_pack_intN(VAR,SIZE)


// Macro: `uvm_pack_sarrayN
//
// Pack a static array of integrals.
//
//| `uvm_pack_sarray(VAR,SIZE)
//
`define uvm_pack_sarrayN(VAR,SIZE) \
    begin \
    foreach (VAR `` [index]) \
      `uvm_pack_intN(VAR[index],SIZE) \
    end


// Macro: `uvm_pack_arrayN
//
// Pack a dynamic array of integrals.
//
//| `uvm_pack_arrayN(VAR,SIZE)
//
`define uvm_pack_arrayN(VAR,SIZE) \
    begin \
    if (packer.use_metadata) \
      `uvm_pack_intN(VAR.size(),32) \
    `uvm_pack_sarrayN(VAR,SIZE) \
    end


// Macro: `uvm_pack_queueN
//
// Pack a queue of integrals.
//
//| `uvm_pack_queueN(VAR,SIZE)
//
`define uvm_pack_queueN(VAR,SIZE) \
   `uvm_pack_arrayN(VAR,SIZE)


//------------------------------
// Group: Packing - No Size Info
//------------------------------

// Macro: `uvm_pack_int
//
// Pack an integral variable without having to also specify the bit size.
//
//| `uvm_pack_int(VAR)
//
`define uvm_pack_int(VAR) \
   `uvm_pack_intN(VAR,$bits(VAR))


// Macro: `uvm_pack_enum
//
// Pack an enumeration value. Packing does not require its type be specified.
//
//| `uvm_pack_enum(VAR)
//
`define uvm_pack_enum(VAR) \
   `uvm_pack_enumN(VAR,$bits(VAR))


// Macro: `uvm_pack_string
//
// Pack a string variable.
//
//| `uvm_pack_string(VAR)
//
`define uvm_pack_string(VAR) \
    begin \
    `uvm_pack_sarrayN(VAR,8) \
    if (packer.use_metadata) \
      `uvm_pack_intN(0,8) \
    end


// Macro: `uvm_pack_real
//
// Pack a variable of type real.
//
//| `uvm_pack_real(VAR)
//
`define uvm_pack_real(VAR) \
   `uvm_pack_intN($realtobits(VAR),64)


// Macro: `uvm_pack_sarray
//
// Pack a static array without having to also specify the bit size
// of its elements.
//
//| `uvm_pack_sarray(VAR)
//
`define uvm_pack_sarray(VAR)  \
   `uvm_pack_sarrayN(VAR,$bits(VAR[0]))


// Macro: `uvm_pack_array
//
// Pack a dynamic array without having to also specify the bit size
// of its elements. Array size must be non-zero.
//
//| `uvm_pack_array(VAR)
//
`define uvm_pack_array(VAR) \
   `uvm_pack_arrayN(VAR,$bits(VAR[0]))


// Macro: `uvm_pack_queue
//
// Pack a queue without having to also specify the bit size
// of its elements. Queue must not be empty.
//
//| `uvm_pack_queue(VAR)
//
`define uvm_pack_queue(VAR) \
   `uvm_pack_queueN(VAR,$bits(VAR[0]))



//------------------------------------------------------------------------------
// Group: Unpacking Macros
//
// The unpacking macros assist users who implement the <uvm_object::do_unpack>
// method. They help ensure that the unpack operation is the exact inverse of
// the pack operation. See also <Packing Macros>.
//
//| virtual function void do_unpack(uvm_packer packer);
//|   `uvm_unpack_enum(cmd,cmd_t)
//|   `uvm_unpack_int(addr)
//|   `uvm_unpack_array(data)
//| endfunction
//
// The 'N' versions of these macros take a explicit size argument.
//------------------------------------------------------------------------------

//----------------------------------
// Group: Unpacking - With Size Info
//----------------------------------

// Macro: `uvm_unpack_intN
//
// Unpack into an integral variable.
//
//| `uvm_unpack_intN(VAR,SIZE)
//
`define uvm_unpack_intN(VAR,SIZE) \
   begin \
   if (packer.big_endian) begin \
     int cnt__ = packer.count + SIZE; \
     uvm_bitstream_t tmp__ = VAR; \
     for (int i=0; i<SIZE; i++) \
       tmp__[i] = packer.m_bits[cnt__ - i - 1]; \
     VAR = tmp__; \
   end \
   else begin \
     VAR = packer.m_bits[packer.count +: SIZE]; \
   end \
   packer.count += SIZE; \
   end


// Macro: `uvm_unpack_enumN
//
// Unpack enum of type ~TYPE~ into ~VAR~.
//
// `uvm_unpack_enumN(VAR,SIZE,TYPE)
//
`define uvm_unpack_enumN(VAR,SIZE,TYPE) \
   begin \
   longint e__; \
   `uvm_unpack_intN(e__,SIZE) \
   VAR = TYPE'(e__); \
   end


// Macro: `uvm_unpack_sarrayN
//
// Unpack a static (fixed) array of integrals.
//
//| `uvm_unpack_sarrayN(VAR,SIZE)
//
`define uvm_unpack_sarrayN(VAR,SIZE) \
    begin \
    foreach (VAR `` [i]) \
      `uvm_unpack_intN(VAR``[i], SIZE) \
    end


// Macro: `uvm_unpack_arrayN
//
// Unpack into a dynamic array of integrals.
//
//| `uvm_unpack_arrayN(VAR,SIZE)
//
`define uvm_unpack_arrayN(VAR,SIZE) \
    begin \
    int sz__; \
    if (packer.use_metadata) \
      `uvm_unpack_intN(sz__,32) \
    VAR = new[sz__]; \
    `uvm_unpack_sarrayN(VAR,SIZE) \
    end


// Macro: `uvm_unpack_queueN
//
// Unpack into a queue of integrals.
//
//| `uvm_unpack_queue(VAR,SIZE)
//
`define uvm_unpack_queueN(VAR,SIZE) \
    begin \
    int sz__; \
    if (packer.use_metadata) \
      `uvm_unpack_intN(sz__,32) \
    while (VAR.size() > sz__) \
      void'(VAR.pop_back()); \
    for (int i=0; i<sz__; i++) \
      `uvm_unpack_intN(VAR[i],SIZE) \
    end


//--------------------------------
// Group: Unpacking - No Size Info
//--------------------------------


// Macro: `uvm_unpack_int
//
// Unpack an integral variable without having to also specify the bit size.
//
//| `uvm_unpack_int(VAR)
//
`define uvm_unpack_int(VAR) \
   `uvm_unpack_intN(VAR,$bits(VAR))


// Macro: `uvm_unpack_enum
//
// Unpack an enumeration value, which requires its type be specified.
//
//| `uvm_unpack_enum(VAR,TYPE)
//
`define uvm_unpack_enum(VAR,TYPE) \
   `uvm_unpack_enumN(VAR,$bits(VAR),TYPE)


// Macro: `uvm_unpack_string
//
// Pack a string variable.
//
//| `uvm_unpack_string(VAR)
//
`ifndef INCA
`define uvm_unpack_string(VAR) \
    begin \
    bit [7:0] chr__; \
    VAR = ""; \
    do begin \
      chr__ = packer.m_bits[packer.count+:8]; \
      packer.count += 8; \
      if (chr__ != 0) \
        VAR = {VAR, string'(chr__)}; \
    end while (chr__ != 0); \
    end
`else
`define uvm_unpack_string(VAR) \
    begin \
    bit [7:0] chr__; \
    VAR = ""; \
    do begin \
      chr__ = packer.m_bits[packer.count+:8]; \
      packer.count += 8; \
      if (chr__ != 0) \
        VAR=$sformatf("%s%s",VAR,chr__); \
    end while (chr__ != 0); \
    end
`endif

// Macro: `uvm_unpack_real
//
// Unpack a variable of type real.
//
//| `uvm_unpack_real(VAR)
//
`define uvm_unpack_real(VAR) \
   begin \
   longint unsigned real_bits64__; \
   `uvm_unpack_intN(real_bits64__,64) \
   VAR = $bitstoreal(real_bits64__); \
   end


// Macro: `uvm_unpack_sarray
//
// Unpack a static array without having to also specify the bit size
// of its elements.
//
// | `uvm_unpack_sarray(VAR)
//
`define uvm_unpack_sarray(VAR)  \
   `uvm_unpack_sarrayN(VAR,$bits(VAR[0]))


// Macro: `uvm_unpack_array
//
// Unpack a dynamic array without having to also specify the bit size
// of its elements. Array size must be non-zero.
//
//| `uvm_unpack_array(VAR)
//
`define uvm_unpack_array(VAR) \
   `uvm_unpack_arrayN(VAR,$bits(VAR[0]))


// Macro: `uvm_unpack_queue
//
// Unpack a queue without having to also specify the bit size
// of its elements. Queue must not be empty.
//
//| `uvm_unpack_queue(VAR)
//
`define uvm_unpack_queue(VAR) \
   `uvm_unpack_queueN(VAR,$bits(VAR[0]))



`endif  // UVM_OBJECT_DEFINES_SVH


