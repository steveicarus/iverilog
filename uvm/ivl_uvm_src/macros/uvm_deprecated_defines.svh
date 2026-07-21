//------------------------------------------------------------------------------
//   Copyright 2010-2011 Mentor Graphics Corporation
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


`ifndef UVM_NO_DEPRECATED

//-----------------------------------------------------------------------------
//
// *** DEPRECATED ***
// Group- Sequence Registration Macros
//
// The sequence-specific macros perform the same function as the set of
// `uvm_object_*_utils macros, except they also set the default sequencer type
// the sequence will run on. 
//-----------------------------------------------------------------------------

`define m_uvm_register_sequence(TYPE_NAME, SEQUENCER) \
  static bit is_registered_with_sequencer = SEQUENCER``::add_typewide_sequence(`"TYPE_NAME`");


// MACRO- `uvm_sequence_utils_begin
//
`define uvm_sequence_utils_begin(TYPE_NAME, SEQUENCER) \
  `m_uvm_register_sequence(TYPE_NAME, SEQUENCER) \
  `uvm_declare_p_sequencer(SEQUENCER) \
  `uvm_object_utils_begin(TYPE_NAME)

// MACRO- `uvm_sequence_utils_end
//
`define uvm_sequence_utils_end \
  `uvm_object_utils_end

// MACRO- `uvm_sequence_utils
//
// The sequence macros can be used in non-parameterized <uvm_sequence #(REQ,RSP)>
// extensions to pre-register the sequence with a given <uvm_sequencer #(REQ,RSP)>
// type.
//
// For sequences that do not use any `uvm_field macros:
//
//|  `uvm_sequence_utils(TYPE_NAME,SQR_TYPE_NAME)
//
// For sequences employing with field macros:
//
//|  `uvm_sequence_utils_begin(TYPE_NAME,SQR_TYPE_NAME)
//|    `uvm_field_* macro invocations here
//|  `uvm_sequence_utils_end
//
// The sequence-specific macros perform the same function as the set of
// `uvm_object_*_utils macros except that they also register the sequence's
// type, TYPE_NAME, with the given sequencer type, SQR_TYPE_NAME, and define
// the p_sequencer variable and m_set_p_sequencer method.
//
// Use `uvm_sequence_utils[_begin] for non-parameterized classes and
// `uvm_sequence_param_utils[_begin] for parameterized classes.

`define uvm_sequence_utils(TYPE_NAME, SEQUENCER) \
  `uvm_sequence_utils_begin(TYPE_NAME,SEQUENCER) \
  `uvm_sequence_utils_end


//-----------------------------------------------------------------------------
//
// *** DEPRECATED ***
//
// Group- Sequencer Registration Macros
//
// The sequencer-specific macros perform the same function as the set of
// `uvm_componenent_*utils macros except that they also declare the plumbing
// necessary for creating the sequencer's sequence library.
//-----------------------------------------------------------------------------

`define uvm_declare_sequence_lib \
  protected bit m_set_sequences_called = 1;    \
  static protected string m_static_sequences[$]; \
  static protected string m_static_remove_sequences[$]; \
  \
  static function bit add_typewide_sequence(string type_name); \
    m_static_sequences.push_back(type_name); \
    return 1; \
  endfunction\
  \
  static function bit remove_typewide_sequence(string type_name); \
    m_static_remove_sequences.push_back(type_name); \
    for (int i = 0; i < m_static_sequences.size(); i++) begin \
      if (m_static_sequences[i] == type_name) \
        m_static_sequences.delete(i); \
    end \
    return 1;\
  endfunction\
  \
  function void uvm_update_sequence_lib();\
    if(this.m_set_sequences_called) begin \
      set_sequences_queue(m_static_sequences); \
      this.m_set_sequences_called = 0;\
    end\
    for (int i = 0; i < m_static_remove_sequences.size(); i++) begin \
      remove_sequence(m_static_remove_sequences[i]); \
    end \
  endfunction\



// MACRO- `uvm_update_sequence_lib
//
// This macro populates the instance-specific sequence library for a sequencer.
// It should be invoked inside the sequencer¿s constructor.

`define uvm_update_sequence_lib \
  m_add_builtin_seqs(0); \
  uvm_update_sequence_lib();


// MACRO- `uvm_update_sequence_lib_and_item
//
// This macro populates the instance specific sequence library for a sequencer,
// and it registers the given ~USER_ITEM~ as an instance override for the simple
// sequence's item variable.
//
// The macro should be invoked inside the sequencer's constructor.

`define uvm_update_sequence_lib_and_item(USER_ITEM) \
  factory.set_inst_override_by_type( \
    uvm_sequence_item::get_type(), USER_ITEM::get_type(), \
    {get_full_name(), "*.item"}); \
  m_add_builtin_seqs(1); \
  uvm_update_sequence_lib();


// MACRO- `uvm_sequencer_utils

`define uvm_sequencer_utils(TYPE_NAME) \
  `uvm_sequencer_utils_begin(TYPE_NAME) \
  `uvm_sequencer_utils_end

// MACRO- `uvm_sequencer_utils_begin

`define uvm_sequencer_utils_begin(TYPE_NAME) \
  `uvm_declare_sequence_lib \
  `uvm_component_utils_begin(TYPE_NAME)

// MACRO- `uvm_sequencer_param_utils

`define uvm_sequencer_param_utils(TYPE_NAME) \
  `uvm_sequencer_param_utils_begin(TYPE_NAME) \
  `uvm_sequencer_utils_end

// MACRO- `uvm_sequencer_param_utils_begin

`define uvm_sequencer_param_utils_begin(TYPE_NAME) \
  `uvm_declare_sequence_lib \
  `uvm_component_param_utils_begin(TYPE_NAME)


// MACRO- `uvm_sequencer_utils_end
//
// The sequencer macros are used in uvm_sequencer-based class declarations
// in one of four ways.
//
// For simple sequencers, no field macros
//
//   `uvm_sequencer_utils(SQR_TYPE_NAME)
//
// For simple sequencers, with field macros
//
//   `uvm_sequencer_utils_begin(SQR_TYPE_NAME)
//     `uvm_field_* macros here
//   `uvm_sequencer_utils_end
//
// For parameterized sequencers, no field macros
//
//   `uvm_sequencer_param_utils(SQR_TYPE_NAME)
//
// For parameterized sequencers, with field macros
//
//   `uvm_sequencer_param_utils_begin(SQR_TYPE_NAME)
//     `uvm_field_* macros here
//   `uvm_sequencer_utils_end
//
// The sequencer-specific macros perform the same function as the set of
// `uvm_componenent_*utils macros except that they also declare the plumbing
// necessary for creating the sequencer's sequence library. This includes:
//
// 1. Declaring the type-based static queue of strings registered on the
//    sequencer type.
//
// 2. Declaring the static function to add strings to item #1 above.
//
// 3. Declaring the static function to remove strings to item #1 above.
//
// 4. Declaring the function to populate the instance specific sequence library
//    for a sequencer.
//
// Use `uvm_sequencer_utils[_begin] for non-parameterized classes and
// `uvm_sequencer_param_utils[_begin] for parameterized classes.

`define uvm_sequencer_utils_end \
  `uvm_component_utils_end



//-----------------------------------------------------------------------------
//
// MACRO- `uvm_package
//
// Use `uvm_package to define the SV package and to create a bogus type to help 
// automate triggering the static initializers of the package.
// Use uvm_end_package to endpackage.
//-----------------------------------------------------------------------------

`define uvm_package(PKG) \
  package PKG; \
  class uvm_bogus_class extends uvm::uvm_sequence;\
  endclass

`define uvm_end_package \
   endpackage


//-----------------------------------------------------------------------------
//
// MACRO- `uvm_sequence_library_package
//
// This macro is used to trigger static initializers in packages. `uvm_package
// creates a bogus type which gets referred to by uvm_sequence_library_package
// to make a package-based variable of the bogus type.
//-----------------------------------------------------------------------------

`define uvm_sequence_library_package(PKG_NAME) \
  import PKG_NAME``::*; \
  PKG_NAME``::uvm_bogus_class M_``PKG_NAME``uvm_bogus_class

`endif // UVM_NO_DEPRECATED
