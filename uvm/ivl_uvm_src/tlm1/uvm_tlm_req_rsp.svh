//
//----------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
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
//----------------------------------------------------------------------


//------------------------------------------------------------------------------
// Title: TLM Channel Classes
//------------------------------------------------------------------------------
// This section defines built-in TLM channel classes.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// CLASS: uvm_tlm_req_rsp_channel #(REQ,RSP)
//
// The uvm_tlm_req_rsp_channel contains a request FIFO of type ~REQ~ and a response
// FIFO of type ~RSP~. These FIFOs can be of any size. This channel is
// particularly useful for dealing with pipelined protocols where the request
// and response are not tightly coupled.
//
// Type parameters:
//
// REQ - Type of the request transactions conveyed by this channel.
// RSP - Type of the reponse transactions conveyed by this channel.
//
//------------------------------------------------------------------------------

class uvm_tlm_req_rsp_channel #(type REQ=int, type RSP=REQ) extends uvm_component;

  typedef uvm_tlm_req_rsp_channel #(REQ, RSP) this_type;

  const static string type_name = "uvm_tlm_req_rsp_channel #(REQ,RSP)";

  // Port: put_request_export
  //
  // The put_export provides both the blocking and non-blocking put interface
  // methods to the request FIFO:
  //
  //|  task put (input T t);
  //|  function bit can_put ();
  //|  function bit try_put (input T t);
  //
  // Any put port variant can connect and send transactions to the request FIFO
  // via this export, provided the transaction types match.

  uvm_put_export #(REQ) put_request_export;


  // Port: get_peek_response_export
  //
  // The get_peek_response_export provides all the blocking and non-blocking get
  // and peek interface methods to the response FIFO:
  //
  //|  task get (output T t);
  //|  function bit can_get ();
  //|  function bit try_get (output T t);
  //|  task peek (output T t);
  //|  function bit can_peek ();
  //|  function bit try_peek (output T t);
  //
  // Any get or peek port variant can connect to and retrieve transactions from
  // the response FIFO via this export, provided the transaction types match.

  uvm_get_peek_export #(RSP) get_peek_response_export;


  // Port: get_peek_request_export
  //
  // The get_peek_export provides all the blocking and non-blocking get and peek
  // interface methods to the response FIFO:
  //
  //|  task get (output T t);
  //|  function bit can_get ();
  //|  function bit try_get (output T t);
  //|  task peek (output T t);
  //|  function bit can_peek ();
  //|  function bit try_peek (output T t);
  //
  // Any get or peek port variant can connect to and retrieve transactions from
  // the response FIFO via this export, provided the transaction types match.


  uvm_get_peek_export #(REQ) get_peek_request_export;


  // Port: put_response_export
  //
  // The put_export provides both the blocking and non-blocking put interface
  // methods to the response FIFO:
  //
  //|  task put (input T t);
  //|  function bit can_put ();
  //|  function bit try_put (input T t);
  //
  // Any put port variant can connect and send transactions to the response FIFO
  // via this export, provided the transaction types match.

  uvm_put_export #(RSP) put_response_export;


  // Port: request_ap
  //
  // Transactions passed via ~put~ or ~try_put~ (via any port connected to the
  // put_request_export) are sent out this port via its write method.
  //
  //|  function void write (T t);
  //
  // All connected analysis exports and imps will receive these transactions.

  uvm_analysis_port #(REQ) request_ap;


  // Port: response_ap
  //
  // Transactions passed via ~put~ or ~try_put~ (via any port connected to the
  // put_response_export) are sent out this port via its write method.
  //
  //|  function void write (T t);
  //
  // All connected analysis exports and imps will receive these transactions.

  uvm_analysis_port   #(RSP) response_ap;


  // Port: master_export
  //
  // Exports a single interface that allows a master to put requests and get or
  // peek responses. It is a combination of the put_request_export and
  // get_peek_response_export.

  uvm_master_imp #(REQ, RSP, this_type, uvm_tlm_fifo #(REQ), uvm_tlm_fifo #(RSP)) master_export;


  // Port: slave_export
  //
  // Exports a single interface that allows a slave to get or peek requests and
  // to put responses. It is a combination of the get_peek_request_export
  // and put_response_export.

  uvm_slave_imp  #(REQ, RSP, this_type, uvm_tlm_fifo #(REQ), uvm_tlm_fifo #(RSP)) slave_export;

  // port aliases for backward compatibility
  uvm_put_export      #(REQ) blocking_put_request_export,
                             nonblocking_put_request_export;
  uvm_get_peek_export #(REQ) get_request_export,
                             blocking_get_request_export,
                             nonblocking_get_request_export,
                             peek_request_export,
                             blocking_peek_request_export,
                             nonblocking_peek_request_export,
                             blocking_get_peek_request_export,
                             nonblocking_get_peek_request_export;

  uvm_put_export      #(RSP) blocking_put_response_export,
                             nonblocking_put_response_export;
  uvm_get_peek_export #(RSP) get_response_export,
                             blocking_get_response_export,
                             nonblocking_get_response_export,
                             peek_response_export,
                             blocking_peek_response_export,
                             nonblocking_peek_response_export,
                             blocking_get_peek_response_export,
                             nonblocking_get_peek_response_export;

  uvm_master_imp #(REQ, RSP, this_type, uvm_tlm_fifo #(REQ), uvm_tlm_fifo #(RSP))
                             blocking_master_export, 
                             nonblocking_master_export;

  uvm_slave_imp  #(REQ, RSP, this_type, uvm_tlm_fifo #(REQ), uvm_tlm_fifo #(RSP))
                             blocking_slave_export, 
                             nonblocking_slave_export;
  // internal fifos
  protected uvm_tlm_fifo #(REQ) m_request_fifo;
  protected uvm_tlm_fifo #(RSP) m_response_fifo;


  // Function: new
  //
  // The ~name~ and ~parent~ are the standard <uvm_component> constructor arguments.
  // The ~parent~ must be null if this component is defined within a static
  // component such as a module, program block, or interface. The last two
  // arguments specify the request and response FIFO sizes, which have default
  // values of 1.

  function new (string name, uvm_component parent=null, 
                int request_fifo_size=1,
                int response_fifo_size=1);

    super.new (name, parent);

    m_request_fifo  = new ("request_fifo",  this, request_fifo_size);
    m_response_fifo = new ("response_fifo", this, response_fifo_size);

    request_ap      = new ("request_ap",  this);
    response_ap     = new ("response_ap", this);
            
    put_request_export       = new ("put_request_export",       this);
    get_peek_request_export  = new ("get_peek_request_export",  this);

    put_response_export      = new ("put_response_export",      this); 
    get_peek_response_export = new ("get_peek_response_export", this);

    master_export   = new ("master_export", this, m_request_fifo, m_response_fifo);
    slave_export    = new ("slave_export",  this, m_request_fifo, m_response_fifo);

    create_aliased_exports();

    set_report_id_action_hier(s_connection_error_id, UVM_NO_ACTION);

  endfunction

  virtual function void connect_phase(uvm_phase phase);
    put_request_export.connect       (m_request_fifo.put_export);
    get_peek_request_export.connect  (m_request_fifo.get_peek_export);
    m_request_fifo.put_ap.connect    (request_ap);
    put_response_export.connect      (m_response_fifo.put_export);
    get_peek_response_export.connect (m_response_fifo.get_peek_export);
    m_response_fifo.put_ap.connect   (response_ap);
  endfunction

  function void create_aliased_exports();
    // request
    blocking_put_request_export         = put_request_export;
    nonblocking_put_request_export      = put_request_export;
    get_request_export                  = get_peek_request_export;
    blocking_get_request_export         = get_peek_request_export;
    nonblocking_get_request_export      = get_peek_request_export;
    peek_request_export                 = get_peek_request_export;
    blocking_peek_request_export        = get_peek_request_export;
    nonblocking_peek_request_export     = get_peek_request_export;
    blocking_get_peek_request_export    = get_peek_request_export;
    nonblocking_get_peek_request_export = get_peek_request_export;
  
    // response
    blocking_put_response_export         = put_response_export;
    nonblocking_put_response_export      = put_response_export;
    get_response_export                  = get_peek_response_export;
    blocking_get_response_export         = get_peek_response_export;
    nonblocking_get_response_export      = get_peek_response_export;
    peek_response_export                 = get_peek_response_export;
    blocking_peek_response_export        = get_peek_response_export;
    nonblocking_peek_response_export     = get_peek_response_export;
    blocking_get_peek_response_export    = get_peek_response_export;
    nonblocking_get_peek_response_export = get_peek_response_export;
  
    // master/slave
    blocking_master_export    = master_export; 
    nonblocking_master_export = master_export;
    blocking_slave_export     = slave_export;
    nonblocking_slave_export  = slave_export;
  endfunction
  
  // get_type_name
  // -------------

  function string get_type_name ();
    return type_name;
  endfunction


  // create
  // ------
  
  function uvm_object create (string name=""); 
    this_type v;
    v=new(name);
    return v;
  endfunction


endclass


//------------------------------------------------------------------------------
//
// CLASS: uvm_tlm_transport_channel #(REQ,RSP)
//
// A uvm_tlm_transport_channel is a <uvm_tlm_req_rsp_channel #(REQ,RSP)> that implements
// the transport interface. It is useful when modeling a non-pipelined bus at
// the transaction level. Because the requests and responses have a tightly
// coupled one-to-one relationship, the request and response FIFO sizes are both
// set to one.
//
//------------------------------------------------------------------------------

class uvm_tlm_transport_channel #(type REQ=int, type RSP=REQ) 
                                     extends uvm_tlm_req_rsp_channel #(REQ, RSP);

  typedef uvm_tlm_transport_channel #(REQ, RSP) this_type;

  // Port: transport_export
  //
  // The put_export provides both the blocking and non-blocking transport
  // interface methods to the response FIFO:
  //
  //|  task transport(REQ request, output RSP response);
  //|  function bit nb_transport(REQ request, output RSP response);
  //
  // Any transport port variant can connect to and send requests and retrieve
  // responses via this export, provided the transaction types match. Upon
  // return, the response argument carries the response to the request.

  uvm_transport_imp #(REQ, RSP, this_type) transport_export;


  // Function: new
  //
  // The ~name~ and ~parent~ are the standard <uvm_component> constructor
  // arguments. The ~parent~ must be null if this component is defined within a
  // statically elaborated construct such as a module, program block, or
  // interface.

  function new (string name, uvm_component parent=null);
    super.new(name, parent, 1, 1);
    transport_export = new("transport_export", this);
  endfunction

  task transport (REQ request, output RSP response );
    this.m_request_fifo.put( request );
    this.m_response_fifo.get( response );
  endtask

  function bit nb_transport (REQ req, output RSP rsp );
    if(this.m_request_fifo.try_put(req)) 
      return this.m_response_fifo.try_get(rsp);
    else
      return 0;
  endfunction

endclass
