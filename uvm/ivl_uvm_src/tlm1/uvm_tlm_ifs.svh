//
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

`define UVM_TASK_ERROR "TLM interface task not implemented"
`define UVM_FUNCTION_ERROR "TLM interface function not implemented"

//-----------------------------------------------------------------------------
//
// CLASS: uvm_tlm_if_base #(T1,T2)
//
// This class declares all of the methods of the TLM API.
//
// Various subsets of these methods are combined to form primitive TLM
// interfaces, which are then paired in various ways to form more abstract
// "combination" TLM interfaces. Components that require a particular interface
// use ports to convey that requirement. Components that provide a particular
// interface use exports to convey its availability.
//
// Communication between components is established by connecting ports to
// compatible exports, much like connecting module signal-level output ports to
// compatible input ports. The difference is that UVM ports and exports bind
// interfaces (groups of methods), not signals and wires. The methods of the
// interfaces so bound pass data as whole transactions (e.g. objects).
// The set of primitve and combination TLM interfaces afford many choices for
// designing components that communicate at the transaction level.
// 
//-----------------------------------------------------------------------------

virtual class uvm_tlm_if_base #(type T1=int, type T2=int);

  // Group: Blocking put

  // Task: put
  //
  // Sends a user-defined transaction of type T. 
  //
  // Components implementing the put method will block the calling thread if
  // it cannot immediately accept delivery of the transaction.

  virtual task put( input T1 t );
    uvm_report_error("put", `UVM_TASK_ERROR, UVM_NONE);
  endtask

  // Group: Blocking get

  // Task: get
  //
  // Provides a new transaction of type T. 
  //
  // The calling thread is blocked if the requested transaction cannot be
  // provided immediately. The new transaction is returned in the provided
  // output argument. 
  //
  // The implementation of get must regard the transaction as consumed.
  // Subsequent calls to get must return a different transaction instance.

  virtual task get( output T2 t );
    uvm_report_error("get", `UVM_TASK_ERROR, UVM_NONE);
  endtask


  // Group: Blocking peek

  // Task: peek
  //
  // Obtain a new transaction without consuming it. 
  //
  // If a transaction is available, then it is written to the provided output
  // argument. If a transaction is not available, then the calling thread is
  // blocked until one is available. 
  //
  // The returned transaction is not consumed. A subsequent peek or get will
  // return the same transaction.

  virtual task peek( output T2 t );
    uvm_report_error("peek", `UVM_TASK_ERROR, UVM_NONE);
  endtask


  // Group: Non-blocking put

  // Function: try_put
  //
  // Sends a transaction of type T, if possible. 
  //
  // If the component is ready to accept the transaction argument, then it does
  // so and returns 1, otherwise it returns 0.

  virtual function bit try_put( input T1 t );
    uvm_report_error("try_put", `UVM_FUNCTION_ERROR, UVM_NONE);
    return 0;
  endfunction


  // Function: can_put
  //
  // Returns 1 if the component is ready to accept the transaction; 0 otherwise.

  virtual function bit can_put();
    uvm_report_error("can_put", `UVM_FUNCTION_ERROR, UVM_NONE);
    return 0;
  endfunction


  // Group: Non-blocking get

  // Function: try_get
  //
  // Provides a new transaction of type T.
  //
  // If a transaction is immediately available, then it is written to the output
  // argument and 1 is returned. Otherwise, the output argument is not modified
  // and 0 is returned.

  virtual function bit try_get( output T2 t );
    uvm_report_error("try_get", `UVM_FUNCTION_ERROR, UVM_NONE);
    return 0;
  endfunction


  // Function: can_get
  //
  // Returns 1 if a new transaction can be provided immediately upon request,
  // 0 otherwise.

  virtual function bit can_get();
    uvm_report_error("can_get", `UVM_FUNCTION_ERROR, UVM_NONE);
    return 0;
  endfunction


  // Group: Non-blocking peek

  // Function: try_peek
  //
  // Provides a new transaction without consuming it. 
  //
  // If available, a transaction is written to the output argument and 1 is
  // returned. A subsequent peek or get will return the same transaction. If a
  // transaction is not available, then the argument is unmodified and 0 is
  // returned.

  virtual function bit try_peek( output T2 t );
    uvm_report_error("try_peek", `UVM_FUNCTION_ERROR, UVM_NONE);
    return 0;
  endfunction


  // Function: can_peek
  //
  // Returns 1 if a new transaction is available; 0 otherwise.

  virtual function bit can_peek();
    uvm_report_error("can_ppeek", `UVM_FUNCTION_ERROR, UVM_NONE);
    return 0;
  endfunction


  // Group: Blocking transport

  // Task: transport
  //
  // Executes the given request and returns the response in the given output
  // argument. The calling thread may block until the operation is complete.

  virtual task transport( input T1 req , output T2 rsp );
    uvm_report_error("transport", `UVM_TASK_ERROR, UVM_NONE);
  endtask


  // Group: Non-blocking transport

  // Task: nb_transport
  //
  // Executes the given request and returns the response in the given output
  // argument. Completion of this operation must occur without blocking.
  //
  // If for any reason the operation could not be executed immediately, then
  // a 0 must be returned; otherwise 1.

  virtual function bit nb_transport(input T1 req, output T2 rsp);
    uvm_report_error("nb_transport", `UVM_FUNCTION_ERROR, UVM_NONE);
    return 0;
  endfunction


  // Group: Analysis

  // Function: write
  //
  // Broadcasts a user-defined transaction of type T to any number of listeners.
  // The operation must complete without blocking. 

  virtual function void write( input T1 t );
    uvm_report_error("write", `UVM_FUNCTION_ERROR, UVM_NONE);
  endfunction

endclass

