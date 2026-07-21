//
//----------------------------------------------------------------------
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
//----------------------------------------------------------------------

//------------------------------------------------------------------------------
// Title: TLM Port Classes
//------------------------------------------------------------------------------
// The following classes define the TLM port classes.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//
// Class: uvm_*_port #(T)
//
// These unidirectional ports are instantiated by components that ~require~,
// or ~use~, the associated interface to convey transactions. A port can
// be connected to any compatible port, export, or imp port. Unless its
// ~min_size~ is 0, a port ~must~ be connected to at least one implementation
// of its assocated interface.
//
// The asterisk in ~uvm_*_port~ is any of the following
//
//|  blocking_put
//|  nonblocking_put
//|  put
//|
//|  blocking_get
//|  nonblocking_get
//|  get
//|
//|  blocking_peek
//|  nonblocking_peek
//|  peek
//|
//|  blocking_get_peek
//|  nonblocking_get_peek
//|  get_peek
//
// Type parameters
//
// T - The type of transaction to be communicated by the export
//
// Ports are connected to interface implementations directly via 
// <uvm_*_imp #(T,IMP)> ports or indirectly via hierarchical connections
// to <uvm_*_port #(T)> and <uvm_*_export #(T)> ports.
//
//------------------------------------------------------------------------------


// Function: new
// 
// The ~name~ and ~parent~ are the standard <uvm_component> constructor arguments.
// The ~min_size~ and ~max_size~ specify the minimum and maximum number of
// interfaces that must have been connected to this port by the end of elaboration.
//
//|  function new (string name, 
//|                uvm_component parent,
//|                int min_size=1,
//|                int max_size=1)

class uvm_blocking_put_port #(type T=int)
  extends uvm_port_base #(uvm_tlm_if_base #(T,T));
  `UVM_PORT_COMMON(`UVM_TLM_BLOCKING_PUT_MASK,"uvm_blocking_put_port")
  `UVM_BLOCKING_PUT_IMP (this.m_if, T, t)
endclass 

class uvm_nonblocking_put_port #(type T=int)
  extends uvm_port_base #(uvm_tlm_if_base #(T,T));
  `UVM_PORT_COMMON(`UVM_TLM_NONBLOCKING_PUT_MASK,"uvm_nonblocking_put_port")
  `UVM_NONBLOCKING_PUT_IMP (this.m_if, T, t)
endclass

class uvm_put_port #(type T=int)
  extends uvm_port_base #(uvm_tlm_if_base #(T,T));
  `UVM_PORT_COMMON(`UVM_TLM_PUT_MASK,"uvm_put_port")
  `UVM_PUT_IMP (this.m_if, T, t)
endclass

class uvm_blocking_get_port #(type T=int)
  extends uvm_port_base #(uvm_tlm_if_base #(T,T));
  `UVM_PORT_COMMON(`UVM_TLM_BLOCKING_GET_MASK,"uvm_blocking_get_port")
  `UVM_BLOCKING_GET_IMP (this.m_if, T, t)
endclass 

class uvm_nonblocking_get_port #(type T=int)
  extends uvm_port_base #(uvm_tlm_if_base #(T,T));
  `UVM_PORT_COMMON(`UVM_TLM_NONBLOCKING_GET_MASK,"uvm_nonblocking_get_port")
  `UVM_NONBLOCKING_GET_IMP (this.m_if, T, t)
endclass

class uvm_get_port #(type T=int)
  extends uvm_port_base #(uvm_tlm_if_base #(T,T));
  `UVM_PORT_COMMON(`UVM_TLM_GET_MASK,"uvm_get_port")
  `UVM_GET_IMP (this.m_if, T, t)
endclass 

class uvm_blocking_peek_port #(type T=int)
  extends uvm_port_base #(uvm_tlm_if_base #(T,T));
  `UVM_PORT_COMMON(`UVM_TLM_BLOCKING_PEEK_MASK,"uvm_blocking_peek_port")
  `UVM_BLOCKING_PEEK_IMP (this.m_if, T, t)
endclass 

class uvm_nonblocking_peek_port #(type T=int)
  extends uvm_port_base #(uvm_tlm_if_base #(T,T));
  `UVM_PORT_COMMON(`UVM_TLM_NONBLOCKING_PEEK_MASK,"uvm_nonblocking_peek_port")
  `UVM_NONBLOCKING_PEEK_IMP (this.m_if, T, t)
endclass

class uvm_peek_port #(type T=int)
  extends uvm_port_base #(uvm_tlm_if_base #(T,T));
  `UVM_PORT_COMMON(`UVM_TLM_PEEK_MASK,"uvm_peek_port")
  `UVM_PEEK_IMP (this.m_if, T, t)
endclass 

class uvm_blocking_get_peek_port #(type T=int)
  extends uvm_port_base #(uvm_tlm_if_base #(T,T));
  `UVM_PORT_COMMON(`UVM_TLM_BLOCKING_GET_PEEK_MASK,"uvm_blocking_get_peek_port")
  `UVM_BLOCKING_GET_PEEK_IMP (this.m_if, T, t)
endclass 

class uvm_nonblocking_get_peek_port #(type T=int)
  extends uvm_port_base #(uvm_tlm_if_base #(T,T));
  `UVM_PORT_COMMON(`UVM_TLM_NONBLOCKING_GET_PEEK_MASK,"uvm_nonblocking_get_peek_port")
  `UVM_NONBLOCKING_GET_PEEK_IMP (this.m_if, T, t)
endclass

class uvm_get_peek_port #(type T=int)
  extends uvm_port_base #(uvm_tlm_if_base #(T,T));
  `UVM_PORT_COMMON(`UVM_TLM_GET_PEEK_MASK,"uvm_get_peek_port")
  `UVM_GET_PEEK_IMP (this.m_if, T, t)
endclass 


//------------------------------------------------------------------------------
//
// Class: uvm_*_port #(REQ,RSP)
//
// These bidirectional ports are instantiated by components that ~require~,
// or ~use~, the associated interface to convey transactions. A port can
// be connected to any compatible port, export, or imp port. Unless its
// ~min_size~ is 0, a port ~must~ be connected to at least one implementation
// of its assocated interface.
//
// The asterisk in ~uvm_*_port~ is any of the following
//
//|  blocking_transport
//|  nonblocking_transport
//|  transport
//|
//|  blocking_master
//|  nonblocking_master
//|  master
//|
//|  blocking_slave
//|  nonblocking_slave
//|  slave
//
// Ports are connected to interface implementations directly via 
// <uvm_*_imp #(REQ,RSP,IMP,REQ_IMP,RSP_IMP)> ports or indirectly via
// hierarchical connections to <uvm_*_port #(REQ,RSP)> and
// <uvm_*_export #(REQ,RSP)> ports.
//
// Type parameters
//
// REQ - The type of request transaction to be communicated by the export
//
// RSP - The type of response transaction to be communicated by the export
//
//------------------------------------------------------------------------------

// Function: new
// 
// The ~name~ and ~parent~ are the standard <uvm_component> constructor arguments.
// The ~min_size~ and ~max_size~ specify the minimum and maximum number of
// interfaces that must have been supplied to this port by the end of elaboration.
//
//   function new (string name, 
//                 uvm_component parent,
//                 int min_size=1,
//                 int max_size=1)


class uvm_blocking_master_port #(type REQ=int, type RSP=REQ)
  extends uvm_port_base #(uvm_tlm_if_base #(REQ, RSP));
  `UVM_PORT_COMMON(`UVM_TLM_BLOCKING_MASTER_MASK,"uvm_blocking_master_port")
  `UVM_BLOCKING_PUT_IMP (this.m_if, REQ, t)
  `UVM_BLOCKING_GET_PEEK_IMP (this.m_if, RSP, t)
endclass 

class uvm_nonblocking_master_port #(type REQ=int, type RSP=REQ)
  extends uvm_port_base #(uvm_tlm_if_base #(REQ, RSP));
  `UVM_PORT_COMMON(`UVM_TLM_NONBLOCKING_MASTER_MASK,"uvm_nonblocking_master_port")
  `UVM_NONBLOCKING_PUT_IMP (this.m_if, REQ, t)
  `UVM_NONBLOCKING_GET_PEEK_IMP (this.m_if, RSP, t)
endclass 

class uvm_master_port #(type REQ=int, type RSP=REQ)
  extends uvm_port_base #(uvm_tlm_if_base #(REQ, RSP));
  `UVM_PORT_COMMON(`UVM_TLM_MASTER_MASK,"uvm_master_port")
  `UVM_PUT_IMP (this.m_if, REQ, t)
  `UVM_GET_PEEK_IMP (this.m_if, RSP, t)
endclass

class uvm_blocking_slave_port #(type REQ=int, type RSP=REQ)
  extends uvm_port_base #(uvm_tlm_if_base #(RSP, REQ));
  `UVM_PORT_COMMON(`UVM_TLM_BLOCKING_SLAVE_MASK,"uvm_blocking_slave_port")
  `UVM_BLOCKING_PUT_IMP (this.m_if, RSP, t)
  `UVM_BLOCKING_GET_PEEK_IMP (this.m_if, REQ, t)
endclass 

class uvm_nonblocking_slave_port #(type REQ=int, type RSP=REQ)
  extends uvm_port_base #(uvm_tlm_if_base #(RSP, REQ));
  `UVM_PORT_COMMON(`UVM_TLM_NONBLOCKING_SLAVE_MASK,"uvm_nonblocking_slave_port")
  `UVM_NONBLOCKING_PUT_IMP (this.m_if, RSP, t)
  `UVM_NONBLOCKING_GET_PEEK_IMP (this.m_if, REQ, t)
endclass 

class uvm_slave_port #(type REQ=int, type RSP=REQ)
  extends uvm_port_base #(uvm_tlm_if_base #(RSP, REQ));
  `UVM_PORT_COMMON(`UVM_TLM_SLAVE_MASK,"uvm_slave_port")
  `UVM_PUT_IMP (this.m_if, RSP, t)
  `UVM_GET_PEEK_IMP (this.m_if, REQ, t)
endclass

class uvm_blocking_transport_port #(type REQ=int, type RSP=REQ)
  extends uvm_port_base #(uvm_tlm_if_base #(REQ, RSP));
  `UVM_PORT_COMMON(`UVM_TLM_BLOCKING_TRANSPORT_MASK,"uvm_blocking_transport_port")
  `UVM_BLOCKING_TRANSPORT_IMP (this.m_if, REQ, RSP, req, rsp)
endclass

class uvm_nonblocking_transport_port #(type REQ=int, type RSP=REQ)
  extends uvm_port_base #(uvm_tlm_if_base #(REQ, RSP));
  `UVM_PORT_COMMON(`UVM_TLM_NONBLOCKING_TRANSPORT_MASK,"uvm_nonblocking_transport_port")
  `UVM_NONBLOCKING_TRANSPORT_IMP (this.m_if, REQ, RSP, req, rsp)
endclass

class uvm_transport_port #(type REQ=int, type RSP=REQ)
  extends uvm_port_base #(uvm_tlm_if_base #(REQ, RSP));
  `UVM_PORT_COMMON(`UVM_TLM_TRANSPORT_MASK,"uvm_transport_port")
  `UVM_TRANSPORT_IMP (this.m_if, REQ, RSP, req, rsp)
endclass

