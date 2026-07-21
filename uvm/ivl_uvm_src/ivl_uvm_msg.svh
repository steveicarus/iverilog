// ========== Copyright Header Begin ==========================
// 
// Project: IVL_UVM
// File: ivl_uvm_pkg.sv
// Author(s): Anirudh Pradyumnan (apseng03@gmail.com)
//            Srinivasan Venkataramanan 
//
// Copyright (c) VerifWorks 2016-2020  All Rights Reserved.
// Contact us via: support@verifworks.com
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES.
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License version 3 as published by the Free Software Foundation.
// 
// This program is distributed in the hope that it will be 
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with this work; if not, write to the Free Software
// 
// ========== Copyright Header End ============================
////////////////////////////////////////////////////////////////////////

  function void uvm_count_info();
    uvm_info_counter++;
  endfunction : uvm_count_info

  function void uvm_count_warn();
    uvm_warn_counter++;
  endfunction : uvm_count_warn

  function void uvm_count_err();
    uvm_err_counter++;
			// For debug $display ("UVM_MAX_QUIT_COUNT: %0d cur_num_err: %0d ",  ivl_uvm_max_quit_count, uvm_err_counter);
		if (uvm_err_counter >= ivl_uvm_max_quit_count) begin
      $display ( "%c[1;31m",27 ) ; // RED color
			$display ("Reached UVM_MAX_QUIT_COUNT: %0d cur_num_err: %0d, ending simulation",  ivl_uvm_max_quit_count, uvm_err_counter);
      $display ( "%c[0m",27 ) ;
      report_summarize ();
			$finish (2);
		end
  endfunction : uvm_count_err

  function void uvm_count_fatal();
    uvm_fatal_counter++;
  endfunction : uvm_count_fatal


  function void report_summarize ();
    int num_errs;

    if (!report_summarize_done) begin : do_it_only_once
      report_summarize_done = 1;
      $display("");
      $display("--- UVM Report Summary ---");
      $display("");
      $display("** Report counts by severity");
      $display ("UVM_INFO : %0d", uvm_info_counter);
      $display ("UVM_WARNING : %0d", uvm_warn_counter);
      $display ("UVM_ERROR : %0d", uvm_err_counter);
      $display ("UVM_FATAL : %0d", uvm_fatal_counter);
  
      num_errs = uvm_err_counter;
  
      if(num_errs > 0) begin : fail
        $display ( "%c[1;31m",27 ) ; // RED color
        $display ("Test FAILED with %0d error(s), look for UVM_ERROR in log file",
                   num_errs);
        $display ( "%c[0m",27 ) ;
      end : fail
      else begin : pass
        $display ( "%c[5;34m",27 ) ; // BLUE color
        $display ( "*** Congratulations! Test PASSED with NO UVM_ERRORs ***" ) ;
        $display ( "%c[0m",27 ) ;
      end : pass
  
    `uvm_info (log_id, 
      "Thanks for using IVL_UVM Package",
      UVM_NONE)
    end : do_it_only_once

  endfunction : report_summarize

  function void uvm_report_info( string id,
                                  string message,
                                  int verbosity,
                                  string filename = "",
                                  int line = 0);
    string msg_str;

    `ifndef IVL_UVM 
        m_rh.report(UVM_INFO, get_full_name(), id, message, verbosity,
                filename, line, this);
    `else
      msg_str = ivl_uvm_compose_message(UVM_INFO, id, message, filename, line); 
      uvm_count_info(); 
      $display (msg_str);
    `endif // IVL_UVM 

  endfunction : uvm_report_info
  // Function: uvm_report_warning

  function void uvm_report_warning( string id,
                                            string message,
                                            int verbosity = UVM_MEDIUM,
                                            string filename = "",
                                            int line = 0);

    string msg_str;
    `ifndef IVL_UVM 
      m_rh.report(UVM_WARNING, get_full_name(), id, message, verbosity, 
                 filename, line, this);
    `else
      msg_str = ivl_uvm_compose_message(UVM_WARNING, id, message, filename, line); 
      uvm_count_warn(); 
      $display (msg_str);
    `endif // IVL_UVM 


  endfunction

  // Function: uvm_report_error

  function void uvm_report_error( string id,
                                          string message,
                                          int verbosity = UVM_LOW,
                                          string filename = "",
                                          int line = 0);
    string msg_str;
    `ifndef IVL_UVM 
      m_rh.report(UVM_ERROR, get_full_name(), id, message, verbosity, 
                 filename, line, this);
    `else
      msg_str = ivl_uvm_compose_message(UVM_ERROR, id, message, filename, line); 
      uvm_count_err (); 
      $display (msg_str);
    `endif // IVL_UVM 

  endfunction

  // Function: uvm_report_fatal
  //
  // These are the primary reporting methods in the UVM. Using these instead
  // of ~$display~ and other ad hoc approaches ensures consistent output and
  // central control over where output is directed and any actions that
  // result. All reporting methods have the same arguments, although each has
  // a different default verbosity:
  //
  //   id        - a unique id for the report or report group that can be used
  //               for identification and therefore targeted filtering. You can
  //               configure an individual report's actions and output file(s)
  //               using this id string.
  //
  //   message   - the message body, preformatted if necessary to a single
  //               string.
  //
  //   verbosity - the verbosity of the message, indicating its relative
  //               importance. If this number is less than or equal to the
  //               effective verbosity level, see <set_report_verbosity_level>,
  //               then the report is issued, subject to the configured action
  //               and file descriptor settings.  Verbosity is ignored for 
  //               warnings, errors, and fatals. However, if a warning, error
  //               or fatal is demoted to an info message using the
  //               <uvm_report_catcher>, then the verbosity is taken into
  //               account.
  //
  //   filename/line - (Optional) The location from which the report was issued.
  //               Use the predefined macros, `__FILE__ and `__LINE__.
  //               If specified, it is displayed in the output.

  function void uvm_report_fatal( string id,
                                          string message,
                                          int verbosity = UVM_NONE,
                                          string filename = "",
                                          int line = 0);
    string msg_str;
    `ifndef IVL_UVM 
      m_rh.report(UVM_FATAL, get_full_name(), id, message, verbosity, 
                 filename, line, this);
    `else
      msg_str = ivl_uvm_compose_message(UVM_FATAL, id, message, filename, line); 
      uvm_count_fatal (); 
      $display (msg_str);
      report_summarize ();
      $finish (2);
    `endif // IVL_UVM 

  endfunction

  function string get_uvm_severity_type_str (uvm_severity severity);
    string res;

    case (severity)
      0 : res = "UVM_INFO";
      1 : res = "UVM_WARNING";
      2 : res = "UVM_ERROR";
      3 : res = "UVM_FATAL";
    endcase 
    return (res);
  endfunction : get_uvm_severity_type_str 

  
function string ivl_uvm_compose_message(
      uvm_severity severity,
      string id,
      string message,
      string filename,
      int    line
      );
    string sv;
    string time_str;
    string line_str;
    string name;

    
    // sv = uvm_severity_type'(severity);
    sv = get_uvm_severity_type_str (severity);
    $swrite(time_str, "%0t", $realtime);
 
    case(1)
      (name == "" && filename == ""):
	       return {sv, " @ ", time_str, " [", id, "] ", message};
      (name != "" && filename == ""):
	       return {sv, " @ ", time_str, ": ", name, " [", id, "] ", message};
      (name == "" && filename != ""):
           begin
               $swrite(line_str, "%0d", line);
		 return {sv, " ",filename, "(", line_str, ")", " @ ", time_str, " [", id, "] ", message};
           end
      (name != "" && filename != ""):
           begin
               $swrite(line_str, "%0d", line);
	         return {sv, " ", filename, "(", line_str, ")", " @ ", time_str, ": ", name, " [", id, "] ", message};
           end
    endcase
  endfunction : ivl_uvm_compose_message

