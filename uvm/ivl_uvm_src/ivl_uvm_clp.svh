//
//------------------------------------------------------------------------------
//   Copyright 2011 Mentor Graphics Corporation
//   Copyright 2011 Cadence Design Systems, Inc. 
//   Copyright 2011 Synopsys, Inc.
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

`ifndef UVM_CMDLINE_PROCESSOR_SV
`define UVM_CMDLINE_PROCESSOR_SV

class uvm_cmd_line_verb;
  string comp_path;
  string id;
  //TBD uvm_verbosity verb;
  int exec_time;
endclass

// Class: uvm_cmdline_processor
//
// This class provides an interface to the command line arguments that 
// were provided for the given simulation.  The class is intended to be
// used as a singleton, but that isn't required.  The generation of the
// data structures which hold the command line argument information 
// happens during construction of the class object.  A global variable 
// called ~uvm_cmdline_proc~ is created at initialization time and may 
// be used to access command line information.
//
// The uvm_cmdline_processor class also provides support for setting various UVM
// variables from the command line such as components' verbosities and configuration
// settings for integral types and strings.  Each of these capablities is described 
// in the Built-in UVM Aware Command Line Arguments section.
//

module ivl_uvm_cmdline_processor;
  import ivl_uvm_pkg::*;

  string m_argv[$]; 
  string m_plus_argv[$];
  string m_uvm_argv[$];

  string ivl_uvm_clp_args [$];

  //`include "ivl_uvm_msg.svh"

  function string uvm_dpi_get_tool_version (); 
    return "Icarus Verilog version 12.0 (devel)";
  endfunction : uvm_dpi_get_tool_version 

  function string uvm_dpi_get_tool_name (); 
    return "Icarus Verilog with IVL_UVM";
  endfunction : uvm_dpi_get_tool_name 


  // Group: Basic Arguments
  
  // Function: get_args
  //
  // This function returns a queue with all of the command line
  // arguments that were used to start the simulation. Note that
  // element 0 of the array will always be the name of the 
  // executable which started the simulation.

  function void get_args (string args[$]);
    ivl_uvm_clp_args = m_argv;
  endfunction

  // Function: get_plusargs
  //
  // This function returns a queue with all of the plus arguments
  // that were used to start the simulation. Plusarguments may be
  // used by the simulator vendor, or may be specific to a company
  // or individiual user. Plusargs never have extra arguments
  // (i.e. if there is a plusarg as the second argument on the
  // command line, the third argument is unrelated); this is not
  // necessarily the case with vendor specific dash arguments.

  function void get_plusargs (string args[$]);
    ivl_uvm_clp_args = m_plus_argv;
  endfunction

  // Function: get_uvmargs
  //
  // This function returns a queue with all of the uvm arguments
  // that were used to start the simulation. An UVM argument is
  // taken to be any argument that starts with a - or + and uses
  // the keyword UVM (case insensitive) as the first three
  // letters of the argument.

  function void get_uvm_args (string args[$]);
    ivl_uvm_clp_args = m_uvm_argv;
  endfunction

  // Function: get_arg_matches
  //
  // This function loads a queue with all of the arguments that
  // match the input expression and returns the number of items
  // that matched. If the input expression is bracketed
  // with //, then it is taken as an extended regular expression 
  // otherwise, it is taken as the beginning of an argument to match.
  // For example:
  //
  //| string myargs[$]
  //| initial begin
  //|    void'(uvm_cmdline_proc.get_arg_matches("+foo",myargs)); //matches +foo, +foobar
  //|                                                            //doesn't match +barfoo
  //|    void'(uvm_cmdline_proc.get_arg_matches("/foo/",myargs)); //matches +foo, +foobar,
  //|                                                             //foo.sv, barfoo, etc.
  //|    void'(uvm_cmdline_proc.get_arg_matches("/^foo.*\.sv",myargs)); //matches foo.sv
  //|                                                                   //and foo123.sv,
  //|                                                                   //not barfoo.sv.

  `ifndef IVL_UVM
  function int get_arg_matches (string match, ref string args[$]);
  `else
  function int get_arg_matches (string match, string unused_args[$]);
  `endif // IVL_UVM

   `ifndef UVM_CMDLINE_NO_DPI
    chandle exp_h = null;
    int len = match.len();
    ivl_uvm_clp_args.delete();
    if((match.len() > 2) && (match[0] == "/") && (match[match.len()-1] == "/")) begin
       match = match.substr(1,match.len()-2);
       exp_h = uvm_dpi_regcomp(match);
       if(exp_h == null) begin
         uvm_report_error("UVM_CMDLINE_PROC", {"Unable to compile the regular expression: ", match}, UVM_NONE);
         return 0;
       end
    end
    foreach (m_argv[i]) begin
      if(exp_h != null) begin
        if(!uvm_dpi_regexec(exp_h, m_argv[i]))
           ivl_uvm_clp_args.push_back(m_argv[i]);
      end
      else if((m_argv[i].len() >= len) && (m_argv[i].substr(0,len - 1) == match))
        ivl_uvm_clp_args.push_back(m_argv[i]);
    end

    if(exp_h != null)
      uvm_dpi_regfree(exp_h);
    `endif

    return ivl_uvm_clp_args.size();
  endfunction


  // Group: Argument Values

  // Function: get_arg_value
  //
  // This function finds the first argument which matches the ~match~ arg and
  // returns the suffix of the argument. This is similar to the $value$plusargs
  // system task, but does not take a formating string. The return value is
  // the number of command line arguments that match the ~match~ string, and
  // ~value~ is the value of the first match.
  
  function int get_arg_value (string match, string value);
    int chars; 
    string lv_argv_s;
    string lv_val_s;
    string lv_argv_substr;
    int lv_argv_len;
    int lv_len_1;

    chars = match.len();
    get_arg_value = 0;
    foreach (m_argv[i]) begin
      // if(m_argv[i].len() >= chars) begin
      lv_argv_s = m_argv[i];
      lv_argv_len = lv_argv_s.len();
	
      if(lv_argv_len >= chars) begin
        // if(m_argv[i].substr(0,chars-1) == match) begin
        lv_argv_substr = lv_argv_s.substr(0,chars-1);
        if(lv_argv_substr == match) begin
          get_arg_value++;
	  if(get_arg_value == 1) begin
            // value = m_argv[i].substr(chars,m_argv[i].len()-1);
	    lv_len_1 = lv_argv_s.len()-1;
            lv_val_s = lv_argv_s.substr(chars,lv_len_1);
	  end
        end
      end
    end
  endfunction

  // Function: get_arg_values
  //
  // This function finds all the arguments which matches the ~match~ arg and
  // returns the suffix of the arguments in a list of values. The return
  // value is the number of matches that were found (it is the same as
  // values.size() ).
  // For example if '+foo=1,yes,on +foo=5,no,off' was provided on the command
  // line and the following code was executed:
  //
  //| string foo_values[$]
  //| initial begin
  //|    void'(uvm_cmdline_proc.get_arg_values("+foo=",foo_values));
  //|
  //
  // The foo_values queue would contain two entries.  These entries are shown
  // here:
  //
  //   0 - "1,yes,on"
  //   1 - "5,no,off"
  //
  // Splitting the resultant string is left to user but using the
  // uvm_split_string() function is recommended.

  function int get_arg_values (string match, string values[$]);
    int chars;

    chars = match.len();
    values.delete();
    `ifndef IVL_UVM
    foreach (m_argv[i]) begin
      if(m_argv[i].len() >= chars) begin
        if(m_argv[i].substr(0,chars-1) == match)
          values.push_back(m_argv[i].substr(chars,m_argv[i].len()-1));
      end
    end
  `endif // IVL_UVM
    return values.size();
  endfunction

  // Group: Tool information

  // Function: get_tool_name
  //
  // Returns the simulation tool that is executing the simlation.
  // This is a vendor specific string.

  function string get_tool_name ();
    return uvm_dpi_get_tool_name();
  endfunction

  // Function: get_tool_version
  //
  // Returns the version of the simulation tool that is executing the simlation.
  // This is a vendor specific string.

  function string  get_tool_version ();
    return uvm_dpi_get_tool_version();
  endfunction

  // constructor

  `ifndef IVL_UVM
  function void ivl_uvm_new (string name = "");
    string s;
    string sub;
    do begin
      s = uvm_dpi_get_next_arg();
      if(s!="") begin
        m_argv.push_back(s);
        if(s[0] == "+") begin
          m_plus_argv.push_back(s);
        end 
        if(s.len() >= 4 && (s[0]=="-" || s[0]=="+")) begin
          sub = s.substr(1,3);
          sub = sub.toupper();
          if(sub == "UVM")
            m_uvm_argv.push_back(s);
        end 
      end
    end while(s!=""); 

    // Group: Command Line Debug

    // Variable: +UVM_DUMP_CMDLINE_ARGS
    //
    // ~+UVM_DUMP_CMDLINE_ARGS~ allows the user to dump all command line arguments to the
    // reporting mechanism.  The output in is tree format.

    // The implementation of this is in uvm_root.

    // Group: Built-in UVM Aware Command Line Arguments

    // Variable: +UVM_TESTNAME
    //
    // ~+UVM_TESTNAME=<class name>~ allows the user to specify which uvm_test (or
    // uvm_component) should be created via the factory and cycled through the UVM phases.
    // If multiple of these settings are provided, the first occurrence is used and a warning
    // is issued for subsequent settings.  For example:
    //
    //| <sim command> +UVM_TESTNAME=read_modify_write_test
    //

    // The implementation of this is in uvm_root since this is procedurally invoked via
    // ovm_root::run_test().

    // Variable: +UVM_VERBOSITY
    //
    // ~+UVM_VERBOSITY=<verbosity>~ allows the user to specify the initial verbosity 
    // for all components.  If multiple of these settings are provided, the first occurrence
    // is used and a warning is issued for subsequent settings.  For example:
    //
    //| <sim command> +UVM_VERBOSITY=UVM_HIGH
    //

    // The implementation of this is in uvm_root since this is procedurally invoked via
    // ovm_root::new().

    // Variable: +uvm_set_verbosity
    //
    // ~+uvm_set_verbosity=<comp>,<id>,<verbosity>,<phase>~ and
    // ~+uvm_set_verbosity=<comp>,<id>,<verbosity>,time,<time>~ allow the users to manipulate the
    // verbosity of specific components at specific phases (and times during the "run" phases)
    // of the simulation.  The ~id~ argument can be either ~_ALL_~ for all IDs or a
    // specific message id.  Wildcarding is not supported for ~id~ due to performance concerns.
    // Settings for non-"run" phases are executed in order of occurrence on the command line.  
    // Settings for "run" phases (times) are sorted by time and then executed in order of 
    // occurrence for settings of the same time.  For example:
    //
    //| <sim command> +uvm_set_verbosity=uvm_test_top.env0.agent1.*,_ALL_,UVM_FULL,time,800
    //

    // Variable: +uvm_set_action
    //
    // ~+uvm_set_action=<comp>,<id>,<severity>,<action>~ provides the equivalent of
    // various uvm_report_object's set_report_*_action APIs.  The special keyword, 
    // ~_ALL_~, can be provided for both/either the ~id~ and/or ~severity~ arguments.  The
    // action can be UVM_NO_ACTION or a | separated list of the other UVM message
    // actions.  For example:
    //
    //| <sim command> +uvm_set_action=uvm_test_top.env0.*,_ALL_,UVM_ERROR,UVM_NO_ACTION
    //

    // Variable: +uvm_set_severity
    //
    // ~+uvm_set_severity=<comp>,<id>,<current severity>,<new severity>~ provides the
    // equivalent of the various uvm_report_object's set_report_*_severity_override APIs. The
    // special keyword, ~_ALL_~, can be provided for both/either the ~id~ and/or
    // ~current severity~ arguments.  For example:
    //
    //| <sim command> +uvm_set_severity=uvm_test_top.env0.*,BAD_CRC,UVM_ERROR,UVM_WARNING
    //

    // Variable: +UVM_TIMEOUT
    //
    // ~+UVM_TIMEOUT=<timeout>,<overridable>~ allows users to change the global timeout of the UVM
    // framework.  The <overridable> argument ('YES' or 'NO') specifies whether user code can subsequently
    // change this value.  If set to 'NO' and the user code tries to change the global timeout value, an
    // warning message will be generated.
    //
    //| <sim command> +UVM_TIMEOUT=200000,NO
    //

    // The implementation of this is in uvm_root.

    // Variable: +UVM_MAX_QUIT_COUNT
    //
    // ~+UVM_MAX_QUIT_COUNT=<count>,<overridable>~ allows users to change max quit count for the report
    // server.  The <overridable> argument ('YES' or 'NO') specifies whether user code can subsequently
    // change this value.  If set to 'NO' and the user code tries to change the max quit count value, an
    // warning message will be generated.
    //
    //| <sim command> +UVM_MAX_QUIT_COUNT=5,NO
    //


    // Variable: +UVM_PHASE_TRACE
    //
    // ~+UVM_PHASE_TRACE~ turns on tracing of phase executions.  Users simply need to put the
    // argument on the command line.

    // Variable: +UVM_OBJECTION_TRACE
    //
    // ~+UVM_OBJECTION_TRACE~ turns on tracing of objection activity.  Users simply need to put the
    // argument on the command line.

    // Variable: +UVM_RESOURCE_DB_TRACE
    //
    // ~+UVM_RESOURCE_DB_TRACE~ turns on tracing of resource DB access.
    // Users simply need to put the argument on the command line.

    // Variable: +UVM_CONFIG_DB_TRACE
    //
    // ~+UVM_CONFIG_DB_TRACE~ turns on tracing of configuration DB access.
    // Users simply need to put the argument on the command line.

    // Variable: +uvm_set_inst_override, +uvm_set_type_override
    //
    // ~+uvm_set_inst_override=<req_type>,<override_type>,<full_inst_path>~ and
    // ~+uvm_set_type_override=<req_type>,<override_type>[,<replace>]~ work
    // like the name based overrides in the factory--factory.set_inst_override_by_name()
    //  and factory.set_type_override_by_name().
    // For uvm_set_type_override, the third argument is 0 or 1 (the default is
    // 1 if this argument is left off); this argument specifies whether previous
    // type overrides for the type should be replaced.  For example:
    //
    //| <sim command> +uvm_set_type_override=eth_packet,short_eth_packet
    //

    // The implementation of this is in uvm_root.

    // Variable: +uvm_set_config_int, +uvm_set_config_string
    //
    // ~+uvm_set_config_int=<comp>,<field>,<value>~ and
    // ~+uvm_set_config_string=<comp>,<field>,<value>~ work like their
    // procedural counterparts: set_config_int() and set_config_string(). For
    // the value of int config settings, 'b (0b), 'o, 'd, 'h ('x or 0x) 
    // as the first two characters of the value are treated as base specifiers
    // for interpreting the base of the number. Size specifiers are not used
    // since SystemVerilog does not allow size specifiers in string to
    // value conversions.  For example:
    //
    //| <sim command> +uvm_set_config_int=uvm_test_top.soc_env,mode,5
    //
    // No equivalent of set_config_object() exists since no way exists to pass an
    // uvm_object into the simulation via the command line.
    //

    // The implementation of this is in uvm_root.

  endfunction
`endif // IVL_UVM

  function bit m_convert_verb(string verb_str, uvm_verbosity verb_enum);
    case (verb_str)
      "NONE"       : begin verb_enum = UVM_NONE;   return 1; end
      "UVM_NONE"   : begin verb_enum = UVM_NONE;   return 1; end
      "LOW"        : begin verb_enum = UVM_LOW;    return 1; end
      "UVM_LOW"    : begin verb_enum = UVM_LOW;    return 1; end
      "MEDIUM"     : begin verb_enum = UVM_MEDIUM; return 1; end
      "UVM_MEDIUM" : begin verb_enum = UVM_MEDIUM; return 1; end
      "HIGH"       : begin verb_enum = UVM_HIGH;   return 1; end
      "UVM_HIGH"   : begin verb_enum = UVM_HIGH;   return 1; end
      "FULL"       : begin verb_enum = UVM_FULL;   return 1; end
      "UVM_FULL"   : begin verb_enum = UVM_FULL;   return 1; end
      "DEBUG"      : begin verb_enum = UVM_DEBUG;  return 1; end
      "UVM_DEBUG"  : begin verb_enum = UVM_DEBUG;  return 1; end
      default      : begin                         return 0; end
    endcase
  endfunction

    // m_do_timeout_settings
  // ---------------------
  
    // Variable: +UVM_MAX_QUIT_COUNT
  function void ivl_uvm_handle_clp_args ();
		int lv_tmp_count;
    lv_tmp_count = `IVL_UVM_VPA ("UVM_MAX_QUIT_COUNT=%d", ivl_uvm_max_quit_count);
	endfunction : ivl_uvm_handle_clp_args 

  function void m_do_timeout_settings();
    string timeout_settings[$];
    string timeout;
    string split_timeout[$];
    int timeout_count;
    time timeout_int;
    string override_spec;
    int lv_int;

    timeout_count = `IVL_UVM_VPA ("UVM_TIMEOUT=%d", timeout_int);

    if (timeout_count ==  0) begin
      ivl_uvm_glb_timeout = 1ms;
      timeout_int = 1;
    end else begin
      timeout = timeout_settings[0];
      timeout = $sformatf ("%0d", timeout_int);
      ivl_uvm_glb_timeout = timeout_int * 1ms;

      if (timeout_count > 1) begin
        string timeout_list;
        string sep;
        for (int i = 0; i < timeout_settings.size(); i++) begin
          if (i != 0)
            sep = "; ";
          timeout_list = {timeout_list, sep, timeout_settings[i]};
        end
        uvm_report_warning("MULTTIMOUT", 
          $sformatf("Multiple (%0d) +UVM_TIMEOUT arguments provided on the command line.  '%s' will be used.  Provided list: %s.", 
          timeout_count, timeout, timeout_list), UVM_NONE);
      end
      uvm_report_info("TIMOUTSET",
        $sformatf("'+UVM_TIMEOUT=%s' provided on the command line is being applied.", timeout), UVM_NONE);
	/*
        lv_int = $sscanf(timeout,"%d,%s",timeout_int,override_spec);
      case(override_spec)
        "YES"   : set_timeout(timeout_int, 1);
        "NO"    : set_timeout(timeout_int, 0);
        default : set_timeout(timeout_int, 1);
      endcase
      */
    end
    ivl_uvm_glb_timeout = timeout_int * ivl_uvm_glb_timeout;
    
      uvm_report_info("TIMOUTSET",
        $sformatf("'+UVM_TIMEOUT=%0d' Global Timeout: %0t", timeout_int, ivl_uvm_glb_timeout), UVM_NONE);
      /*
    uvm_report_info("TIMOUTSET",
        $sformatf("ivl_uvm_glb_timeout: %0d", ivl_uvm_glb_timeout), UVM_NONE)
*/
  endfunction : m_do_timeout_settings

  initial begin
    $timeformat (-9, 3, " ns", 3);
    #1;
    `g2u_display ("CLP")
    m_do_timeout_settings();
    ivl_uvm_handle_clp_args ();

    #(ivl_uvm_glb_timeout);
    `uvm_error ("IVL_UVM", 
        $sformatf ("Reached Timeout value of: %0t ", ivl_uvm_glb_timeout))
    `g2u_display ("Ending the Simulation; Check for hanging threads/inactivity in your simulation run")	
    // report_summarize();
    $finish (2);
  end
  final begin
	 	report_summarize();
	end

endmodule : ivl_uvm_cmdline_processor

`endif //UVM_CMDLINE_PROC_PKG_SV

