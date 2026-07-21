// ========== Copyright Header Begin ==========================
// 
// Project: IVL_UVM
// File: ivl_uvm_types.svh
// Author(s): Srinivasan Venkataramanan 
// Modified from Accellera UVM 1.1d code base
//
// Copyright (c) VerifWorks 2016-2020  All Rights Reserved.
// Contact us via: support@verifworks.com
// 
// ========== Copyright Header End ============================
////////////////////////////////////////////////////////////////////////
//
//------------------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc. 
//   Copyright 2010 Synopsys, Inc.
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

//This bit marks where filtering should occur to remove uvm stuff from a
//scope
bit uvm_start_uvm_declarations = 1;

//------------------------------------------------------------------------------
//
// Section: Types and Enumerations
//
//------------------------------------------------------------------------------

//------------------------
// Group: Field automation
//------------------------

// Macro: `UVM_MAX_STREAMBITS
//
// Defines the maximum bit vector size for integral types.

`ifndef UVM_MAX_STREAMBITS
`define UVM_MAX_STREAMBITS 4096
`endif


// Macro: `UVM_PACKER_MAX_BYTES
//
// Defines the maximum bytes to allocate for packing an object using
// the <uvm_packer>. Default is <`UVM_MAX_STREAMBITS>, in ~bytes~.

`ifndef UVM_PACKER_MAX_BYTES
  `define UVM_PACKER_MAX_BYTES UVM_STREAMBITS
`endif

parameter UVM_STREAMBITS = `UVM_MAX_STREAMBITS; 


// Macro: `UVM_DEFAULT_TIMEOUT
//
// The default timeout for simulation, if not overridden by
// <uvm_root::set_timeout> or <+UVM_TIMEOUT>
//

`define UVM_DEFAULT_TIMEOUT 9200s

// Type: uvm_bitstream_t
//
// The bitstream type is used as a argument type for passing integral values
// in such methods as set_int_local, get_int_local, get_config_int, report,
// pack and unpack. 

typedef logic signed [UVM_STREAMBITS-1:0] uvm_bitstream_t;



// Enum: uvm_radix_enum
//
// Specifies the radix to print or record in.
//
// UVM_BIN       - Selects binary (%b) format
// UVM_DEC       - Selects decimal (%d) format
// UVM_UNSIGNED  - Selects unsigned decimal (%u) format
// UVM_OCT       - Selects octal (%o) format
// UVM_HEX       - Selects hexidecimal (%h) format
// UVM_STRING    - Selects string (%s) format
// UVM_TIME      - Selects time (%t) format
// UVM_ENUM      - Selects enumeration value (name) format

typedef enum {
   UVM_BIN       = 'h1000000,
   UVM_DEC       = 'h2000000,
   UVM_UNSIGNED  = 'h3000000,
   UVM_UNFORMAT2 = 'h4000000,
   UVM_UNFORMAT4 = 'h5000000,
   UVM_OCT       = 'h6000000,
   UVM_HEX       = 'h7000000,
   UVM_STRING    = 'h8000000,
   UVM_TIME      = 'h9000000,
   UVM_ENUM      = 'ha000000,
   UVM_REAL      = 'hb000000,
   UVM_REAL_DEC  = 'hc000000,
   UVM_REAL_EXP  = 'hd000000,
   UVM_NORADIX   = 0
} uvm_radix_enum;

parameter UVM_RADIX = 'hf000000; //4 bits setting the radix


// Function- uvm_radix_to_string

function string uvm_radix_to_string(uvm_radix_enum radix);
  case(radix)
    UVM_BIN:        return "b";
    UVM_OCT:        return "o";
    UVM_DEC:        return "d";
    UVM_HEX:        return "h";
    UVM_UNSIGNED:   return "u";
    UVM_UNFORMAT2:  return "u";
    UVM_UNFORMAT4:  return "z";
    UVM_STRING:     return "s";
    UVM_TIME:       return "t";
    UVM_ENUM:       return "s";
    UVM_REAL:       return "g";
    UVM_REAL_DEC:   return "f";
    UVM_REAL_EXP:   return "e";
    default:        return "x"; //hex
  endcase
endfunction


// Enum: uvm_recursion_policy_enum
//
// Specifies the policy for copying objects.
//
// UVM_DEEP      - Objects are deep copied (object must implement copy method)
// UVM_SHALLOW   - Objects are shallow copied using default SV copy.
// UVM_REFERENCE - Only object handles are copied.

typedef enum { 
  UVM_DEFAULT_POLICY = 0, 
  UVM_DEEP           = 'h400, 
  UVM_SHALLOW        = 'h800, 
  UVM_REFERENCE      = 'h1000
 } uvm_recursion_policy_enum;


// Enum: uvm_active_passive_enum
//
// Convenience value to define whether a component, usually an agent,
// is in "active" mode or "passive" mode.

typedef enum bit { UVM_PASSIVE=0, UVM_ACTIVE=1 } uvm_active_passive_enum;


// Parameter: `uvm_field_* macro flags
//
// Defines what operations a given field should be involved in.
// Bitwise OR all that apply.
//
// UVM_DEFAULT   - All field operations turned on
// UVM_COPY      - Field will participate in <uvm_object::copy>
// UVM_COMPARE   - Field will participate in <uvm_object::compare>
// UVM_PRINT     - Field will participate in <uvm_object::print>
// UVM_RECORD    - Field will participate in <uvm_object::record>
// UVM_PACK      - Field will participate in <uvm_object::pack>
//
// UVM_NOCOPY    - Field will not participate in <uvm_object::copy>
// UVM_NOCOMPARE - Field will not participate in <uvm_object::compare>
// UVM_NOPRINT   - Field will not participate in <uvm_object::print>
// UVM_NORECORD  - Field will not participate in <uvm_object::record>
// UVM_NOPACK    - Field will not participate in <uvm_object::pack>
//
// UVM_DEEP      - Object field will be deep copied
// UVM_SHALLOW   - Object field will be shallow copied
// UVM_REFERENCE - Object field will copied by reference
//
// UVM_READONLY  - Object field will NOT be automatically configured.


parameter UVM_MACRO_NUMFLAGS    = 17;
//A=ABSTRACT Y=PHYSICAL
//F=REFERENCE, S=SHALLOW, D=DEEP
//K=PACK, R=RECORD, P=PRINT, M=COMPARE, C=COPY
//--------------------------- AYFSD K R P M C
parameter UVM_DEFAULT     = 'b000010101010101;
parameter UVM_ALL_ON      = 'b000000101010101;
parameter UVM_FLAGS_ON    = 'b000000101010101;
parameter UVM_FLAGS_OFF   = 0;

//Values are or'ed into a 32 bit value
//and externally
parameter UVM_COPY         = (1<<0);
parameter UVM_NOCOPY       = (1<<1);
parameter UVM_COMPARE      = (1<<2);
parameter UVM_NOCOMPARE    = (1<<3);
parameter UVM_PRINT        = (1<<4);
parameter UVM_NOPRINT      = (1<<5);
parameter UVM_RECORD       = (1<<6);
parameter UVM_NORECORD     = (1<<7);
parameter UVM_PACK         = (1<<8);
parameter UVM_NOPACK       = (1<<9);
//parameter UVM_DEEP         = (1<<10);
//parameter UVM_SHALLOW      = (1<<11);
//parameter UVM_REFERENCE    = (1<<12);
parameter UVM_PHYSICAL     = (1<<13);
parameter UVM_ABSTRACT     = (1<<14);
parameter UVM_READONLY     = (1<<15);
parameter UVM_NODEFPRINT   = (1<<16);

//Extra values that are used for extra methods
parameter UVM_MACRO_EXTRAS  = (1<<UVM_MACRO_NUMFLAGS);
parameter UVM_FLAGS        = UVM_MACRO_EXTRAS+1;
parameter UVM_UNPACK       = UVM_MACRO_EXTRAS+2;
parameter UVM_CHECK_FIELDS = UVM_MACRO_EXTRAS+3;
parameter UVM_END_DATA_EXTRA = UVM_MACRO_EXTRAS+4;


//Get and set methods (in uvm_object). Used by the set/get* functions
//to tell the object what operation to perform on the fields.
parameter UVM_START_FUNCS  = UVM_END_DATA_EXTRA+1;
parameter UVM_SET           = UVM_START_FUNCS+1;
parameter UVM_SETINT        = UVM_SET;
parameter UVM_SETOBJ        = UVM_START_FUNCS+2;
parameter UVM_SETSTR        = UVM_START_FUNCS+3;
parameter UVM_END_FUNCS     = UVM_SETSTR;

//Global string variables
string uvm_aa_string_key;



//-----------------
// Group: Reporting
//-----------------

// Enum: uvm_severity
//
// Defines all possible values for report severity.
//
//   UVM_INFO    - Informative messsage.
//   UVM_WARNING - Indicates a potential problem.
//   UVM_ERROR   - Indicates a real problem. Simulation continues subject
//                 to the configured message action.
//   UVM_FATAL   - Indicates a problem from which simulation can not
//                 recover. Simulation exits via $finish after a #0 delay.

typedef bit [1:0] uvm_severity;

`ifdef IVL_UVM
typedef enum bit [1:0] 
`else
typedef enum uvm_severity
`endif
{
  UVM_INFO,
  UVM_WARNING,
  UVM_ERROR,
  UVM_FATAL
} uvm_severity_type;


// Enum: uvm_action
//
// Defines all possible values for report actions. Each report is configured
// to execute one or more actions, determined by the bitwise OR of any or all
// of the following enumeration constants.
//
//   UVM_NO_ACTION - No action is taken
//   UVM_DISPLAY   - Sends the report to the standard output
//   UVM_LOG       - Sends the report to the file(s) for this (severity,id) pair
//   UVM_COUNT     - Counts the number of reports with the COUNT attribute.
//                   When this value reaches max_quit_count, the simulation terminates
//   UVM_EXIT      - Terminates the simulation immediately.
//   UVM_CALL_HOOK - Callback the report hook methods 
//   UVM_STOP      - Causes ~$stop~ to be executed, putting the simulation into
//                   interactive mode.


typedef int uvm_action;

typedef enum
{
  UVM_NO_ACTION = 'b000000,
  UVM_DISPLAY   = 'b000001,
  UVM_LOG       = 'b000010,
  UVM_COUNT     = 'b000100,
  UVM_EXIT      = 'b001000,
  UVM_CALL_HOOK = 'b010000,
  UVM_STOP      = 'b100000
} uvm_action_type;


// Enum: uvm_verbosity
//
// Defines standard verbosity levels for reports.
//
//  UVM_NONE   - Report is always printed. Verbosity level setting can not
//               disable it.
//  UVM_LOW    - Report is issued if configured verbosity is set to UVM_LOW
//               or above.
//  UVM_MEDIUM - Report is issued if configured verbosity is set to UVM_MEDIUM
//               or above.
//  UVM_HIGH   - Report is issued if configured verbosity is set to UVM_HIGH
//               or above.
//  UVM_FULL   - Report is issued if configured verbosity is set to UVM_FULL
//               or above.

typedef enum
{
  UVM_NONE   = 0,
  UVM_LOW    = 100,
  UVM_MEDIUM = 200,
  UVM_HIGH   = 300,
  UVM_FULL   = 400,
  UVM_DEBUG  = 500
} uvm_verbosity;


typedef int UVM_FILE;


//-----------------
// Group: Port Type
//-----------------

// Enum: uvm_port_type_e
//
// Specifies the type of port
//
// UVM_PORT           - The port requires the interface that is its type
//                      parameter.
// UVM_EXPORT         - The port provides the interface that is its type
//                      parameter via a connection to some other export or
//                      implementation.
// UVM_IMPLEMENTATION - The port provides the interface that is its type
//                      parameter, and it is bound to the component that
//                      implements the interface.

typedef enum
{
  UVM_PORT ,
  UVM_EXPORT ,
  UVM_IMPLEMENTATION
} uvm_port_type_e;


//-----------------
// Group: Sequences
//-----------------

// Enum: uvm_sequencer_arb_mode
//
// Specifies a sequencer's arbitration mode
//
// SEQ_ARB_FIFO          - Requests are granted in FIFO order (default)
// SEQ_ARB_WEIGHTED      - Requests are granted randomly by weight
// SEQ_ARB_RANDOM        - Requests are granted randomly
// SEQ_ARB_STRICT_FIFO   - Requests at highest priority granted in fifo order
// SEQ_ARB_STRICT_RANDOM - Requests at highest priority granted in randomly
// SEQ_ARB_USER          - Arbitration is delegated to the user-defined 
//                         function, user_priority_arbitration. That function
//                         will specify the next sequence to grant.


typedef enum
{
  SEQ_ARB_FIFO,
  SEQ_ARB_WEIGHTED,
  SEQ_ARB_RANDOM,
  SEQ_ARB_STRICT_FIFO,
  SEQ_ARB_STRICT_RANDOM,
  SEQ_ARB_USER
} uvm_sequencer_arb_mode;


typedef uvm_sequencer_arb_mode SEQ_ARB_TYPE; // backward compat


// Enum: uvm_sequence_state_enum
//
// Defines current sequence state
//
// CREATED            - The sequence has been allocated.
// PRE_START          - The sequence is started and the
//                      <uvm_sequence_base::pre_start()> task is
//                      being executed.
// PRE_BODY           - The sequence is started and the
//                      <uvm_sequence_base::pre_body()> task is
//                      being executed.
// BODY               - The sequence is started and the
//                      <uvm_sequence_base::body()> task is
//                      being executed.
// ENDED              - The sequence has completed the execution of the 
//                      <uvm_sequence_base::body()> task.
// POST_BODY          - The sequence is started and the
//                      <uvm_sequence_base::post_body()> task is
//                      being executed.
// POST_START         - The sequence is started and the
//                      <uvm_sequence_base::post_start()> task is
//                      being executed.
// STOPPED            - The sequence has been forcibly ended by issuing a
//                      <uvm_sequence_base::kill()> on the sequence.
// FINISHED           - The sequence is completely finished executing.

typedef enum
{
  CREATED   = 1,
  PRE_START = 2,
  PRE_BODY  = 4,
  BODY      = 8,
  POST_BODY = 16,
  POST_START= 32,
  ENDED     = 64,
  STOPPED   = 128,
  FINISHED  = 256
} uvm_sequence_state;

typedef uvm_sequence_state uvm_sequence_state_enum; // backward compat


// Enum: uvm_sequence_lib_mode
//
// Specifies the random selection mode of a sequence library
//
// UVM_SEQ_LIB_RAND  - Random sequence selection
// UVM_SEQ_LIB_RANDC - Random cyclic sequence selection
// UVM_SEQ_LIB_ITEM  - Emit only items, no sequence execution
// UVM_SEQ_LIB_USER  - Apply a user-defined random-selection algorithm

typedef enum
{
  UVM_SEQ_LIB_RAND,
  UVM_SEQ_LIB_RANDC,
  UVM_SEQ_LIB_ITEM,
  UVM_SEQ_LIB_USER
} uvm_sequence_lib_mode;



//---------------
// Group: Phasing
//---------------

// Enum: uvm_phase_type
//
// This is an attribute of a <uvm_phase> object which defines the phase
// type. 
//
//   UVM_PHASE_IMP      - The phase object is used to traverse the component
//                        hierarchy and call the component phase method as
//                        well as the ~phase_started~ and ~phase_ended~ callbacks.
//                        These nodes are created by the phase macros,
//                        `uvm_builtin_task_phase, `uvm_builtin_topdown_phase,
//                        and `uvm_builtin_bottomup_phase. These nodes represent
//                        the phase type, i.e. uvm_run_phase, uvm_main_phase.
//
//   UVM_PHASE_NODE     - The object represents a simple node instance in
//                        the graph. These nodes will contain a reference to
//                        their corresponding IMP object. 
//
//   UVM_PHASE_SCHEDULE - The object represents a portion of the phasing graph,
//                        typically consisting of several NODE types, in series,
//                        parallel, or both.
//
//   UVM_PHASE_TERMINAL - This internal object serves as the termination NODE
//                        for a SCHEDULE phase object.
//
//   UVM_PHASE_DOMAIN   - This object represents an entire graph segment that
//                        executes in parallel with the 'run' phase.
//                        Domains may define any network of NODEs and
//                        SCHEDULEs. The built-in domain, ~uvm~, consists
//                        of a single schedule of all the run-time phases,
//                        starting with ~pre_reset~ and ending with
//                        ~post_shutdown~.
//
typedef enum { UVM_PHASE_IMP,
               UVM_PHASE_NODE,
               UVM_PHASE_TERMINAL,
               UVM_PHASE_SCHEDULE,
               UVM_PHASE_DOMAIN,
               UVM_PHASE_GLOBAL
} uvm_phase_type;


// Enum: uvm_phase_state
// ---------------------
//
// The set of possible states of a phase. This is an attribute of a schedule
// node in the graph, not of a phase, to maintain independent per-domain state
//
//   UVM_PHASE_DORMANT -  Nothing has happened with the phase in this domain.
//
//   UVM_PHASE_SCHEDULED - At least one immediate predecessor has completed.
//              Scheduled phases block until all predecessors complete or
//              until a jump is executed.
//
//   UVM_PHASE_SYNCING - All predecessors complete, checking that all synced
//              phases (e.g. across domains) are at or beyond this point
//
//   UVM_PHASE_STARTED - phase ready to execute, running phase_started() callback
//
//   UVM_PHASE_EXECUTING - An executing phase is one where the phase callbacks are
//              being executed. It's process is tracked by the phaser.
//
//   UVM_PHASE_READY_TO_END - no objections remain in this phase or in any
//              predecessors of its successors or in any sync'd phases. This 
//              state indicates an opportunity for any phase that needs extra  
//              time for a clean exit to raise an objection, thereby causing a 
//              return to UVM_PHASE_EXECUTING.  If no objection is raised, state
//              will transition to UVM_PHASE_ENDED after a delta cycle.
//              (An example of predecessors of successors: The successor to
//              phase 'run' is 'extract', whose predecessors are 'run' and 
//              'post_shutdown'. Therefore, 'run' will go to this state when
//              both its objections and those of 'post_shutdown' are all dropped.
//
//   UVM_PHASE_ENDED - phase completed execution, now running phase_ended() callback
//
//   UVM_PHASE_CLEANUP - all processes related to phase are being killed
//
//   UVM_PHASE_DONE - A phase is done after it terminated execution.  Becoming
//              done may enable a waiting successor phase to execute.
//
//    The state transitions occur as follows:
//
//|   DORMANT -> SCHED -> SYNC -> START -> EXEC -> READY -> END -> CLEAN -> DONE
//|      ^                                                            |
//|      |                      <-- jump_to                           v
//|      +------------------------------------------------------------+

   typedef enum { UVM_PHASE_DORMANT      = 1,
                  UVM_PHASE_SCHEDULED    = 2,
                  UVM_PHASE_SYNCING      = 4,
                  UVM_PHASE_STARTED      = 8,
                  UVM_PHASE_EXECUTING    = 16,
                  UVM_PHASE_READY_TO_END = 32,
                  UVM_PHASE_ENDED        = 64,
                  UVM_PHASE_CLEANUP      = 128,
                  UVM_PHASE_DONE         = 256,
                  UVM_PHASE_JUMPING      = 512
                  } uvm_phase_state;



// Enum: uvm_phase_transition
//
// These are the phase state transition for callbacks which provide
// additional information that may be useful during callbacks
//
// UVM_COMPLETED   - the phase completed normally
// UVM_FORCED_STOP - the phase was forced to terminate prematurely
// UVM_SKIPPED     - the phase was in the path of a forward jump
// UVM_RERUN       - the phase was in the path of a backwards jump
//
typedef enum { UVM_COMPLETED   = 'h01, 
               UVM_FORCED_STOP = 'h02,
               UVM_SKIPPED     = 'h04, 
               UVM_RERUN       = 'h08   
} uvm_phase_transition;



// Enum: uvm_wait_op
//
// Specifies the operand when using methods like <uvm_phase::wait_for_state>.
//
// UVM_EQ  - equal
// UVM_NE  - not equal
// UVM_LT  - less than
// UVM_LTE - less than or equal to
// UVM_GT  - greater than
// UVM_GTE - greater than or equal to
//
typedef enum { UVM_LT,
               UVM_LTE,
               UVM_NE,
               UVM_EQ,
               UVM_GT,
               UVM_GTE
} uvm_wait_op;


//------------------
// Group: Objections
//------------------

// Enum: uvm_objection_event
//
// Enumerated the possible objection events one could wait on. See
// <uvm_objection::wait_for>.
//
// UVM_RAISED      - an objection was raised
// UVM_DROPPED     - an objection was raised
// UVM_ALL_DROPPED - all objections have been dropped
//
typedef enum { UVM_RAISED      = 'h01, 
               UVM_DROPPED     = 'h02,
               UVM_ALL_DROPPED = 'h04
} uvm_objection_event;



//------------------------------
// Group: Default Policy Classes
//------------------------------
//
// Policy classes copying, comparing, packing, unpacking, and recording
// <uvm_object>-based objects.


typedef class uvm_printer;
typedef class uvm_table_printer;
typedef class uvm_tree_printer;
typedef class uvm_line_printer;
typedef class uvm_comparer;
typedef class uvm_packer;
typedef class uvm_recorder;

// Variable: uvm_default_table_printer
//
// The table printer is a global object that can be used with
// <uvm_object::do_print> to get tabular style printing.

`ifndef IVL_UVM
uvm_table_printer uvm_default_table_printer = new();
`endif


// Variable: uvm_default_tree_printer
//
// The tree printer is a global object that can be used with
// <uvm_object::do_print> to get multi-line tree style printing.

`ifndef IVL_UVM
uvm_tree_printer uvm_default_tree_printer  = new();
`endif


// Variable: uvm_default_line_printer
//
// The line printer is a global object that can be used with
// <uvm_object::do_print> to get single-line style printing.

`ifndef IVL_UVM
uvm_line_printer uvm_default_line_printer  = new();
`endif


// Variable: uvm_default_printer
//
// The default printer policy. Used when calls to <uvm_object::print>
// or <uvm_object::sprint> do not specify a printer policy.
//
// The default printer may be set to any legal <uvm_printer> derived type,
// including the global line, tree, and table printers described above.

`ifndef IVL_UVM
uvm_printer uvm_default_printer = uvm_default_table_printer;
`endif


// Variable: uvm_default_packer
//
// The default packer policy. Used when calls to <uvm_object::pack>
// and <uvm_object::unpack> do not specify a packer policy.

`ifndef IVL_UVM
uvm_packer uvm_default_packer = new();
`endif


// Variable: uvm_default_comparer
//
//
// The default compare policy. Used when calls to <uvm_object::compare>
// do not specify a comparer policy.

`ifndef IVL_UVM
uvm_comparer uvm_default_comparer = new(); // uvm_comparer::init();
`endif


// Variable: uvm_default_recorder
//
// The default recording policy. Used when calls to <uvm_object::record>
// do not specify a recorder policy.

`ifndef IVL_UVM
uvm_recorder uvm_default_recorder = new();
`endif





