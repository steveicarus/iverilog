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

typedef class uvm_tlm_event;

//------------------------------------------------------------------------------
//
// Title: TLM FIFO Classes
//
// This section defines TLM-based FIFO classes. 
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Class: uvm_tlm_fifo
//
// This class provides storage of transactions between two independently running
// processes. Transactions are put into the FIFO via the ~put_export~. 
// transactions are fetched from the FIFO in the order they arrived via the
// ~get_peek_export~. The ~put_export~ and ~get_peek_export~ are inherited from
// the <uvm_tlm_fifo_base #(T)> super class, and the interface methods provided by
// these exports are defined by the <uvm_tlm_if_base #(T1,T2)> class.
//
//------------------------------------------------------------------------------

class uvm_tlm_fifo #(type T=int) extends uvm_tlm_fifo_base #(T);

  const static string type_name = "uvm_tlm_fifo #(T)";

  local mailbox #( T ) m;
  local int m_size;
  protected int m_pending_blocked_gets;


  // Function: new
  //
  // The ~name~ and ~parent~ are the normal uvm_component constructor arguments. 
  // The ~parent~ should be null if the <uvm_tlm_fifo> is going to be used in a
  // statically elaborated construct (e.g., a module). The ~size~ indicates the
  // maximum size of the FIFO; a value of zero indicates no upper bound.

  function new(string name, uvm_component parent = null, int size = 1);
    super.new(name, parent);
    m = new( size );
    m_size = size;
  endfunction

  virtual function string get_type_name();
    return type_name;
  endfunction


  // Function: size
  //
  // Returns the capacity of the FIFO-- that is, the number of entries
  // the FIFO is capable of holding. A return value of 0 indicates the
  // FIFO capacity has no limit.

  virtual function int size();
    return m_size;
  endfunction
 

  // Function: used
  //
  // Returns the number of entries put into the FIFO.

  virtual function int used();
    return m.num();
  endfunction


  // Function: is_empty
  //
  // Returns 1 when there are no entries in the FIFO, 0 otherwise.

  virtual function bit is_empty();
    return (m.num() == 0);
  endfunction
 

  // Function: is_full
  //
  // Returns 1 when the number of entries in the FIFO is equal to its <size>,
  // 0 otherwise.

  virtual function bit is_full();
    return (m_size != 0) && (m.num() == m_size);
  endfunction
 


  virtual task put( input T t );
    m.put( t );
    put_ap.write( t );
  endtask

  virtual task get( output T t );
    m_pending_blocked_gets++;
    m.get( t );
    m_pending_blocked_gets--;
    get_ap.write( t );
  endtask
  
  virtual task peek( output T t );
    m.peek( t );
  endtask
   
  virtual function bit try_get( output T t );
    if( !m.try_get( t ) ) begin
      return 0;
    end

    get_ap.write( t );
    return 1;
  endfunction 
  
  virtual function bit try_peek( output T t );
    if( !m.try_peek( t ) ) begin
      return 0;
    end
    return 1;
  endfunction

  virtual function bit try_put( input T t );
    if( !m.try_put( t ) ) begin
      return 0;
    end
  
    put_ap.write( t );
    return 1;
  endfunction  

  virtual function bit can_put();
    return m_size == 0 || m.num() < m_size;
  endfunction  

  virtual function bit can_get();
    return m.num() > 0 && m_pending_blocked_gets == 0;
  endfunction
  
  virtual function bit can_peek();
    return m.num() > 0;
  endfunction


  // Function: flush
  //
  // Removes all entries from the FIFO, after which <used> returns 0
  // and <is_empty> returns 1.

  virtual function void flush();
    T t;
    bit r;

    r = 1; 
    while( r ) r = try_get( t ) ;
    
    if( m.num() > 0 && m_pending_blocked_gets != 0 ) begin
      uvm_report_error("flush failed" ,
		       "there are blocked gets preventing the flush", UVM_NONE);
    end
  
  endfunction
 
endclass 


//------------------------------------------------------------------------------
//
// Class: uvm_tlm_analysis_fifo
//
// An analysis_fifo is a <uvm_tlm_fifo> with an unbounded size and a write interface.
// It can be used any place a <uvm_analysis_imp> is used. Typical usage is
// as a buffer between an <uvm_analysis_port> in an initiator component
// and TLM1 target component.
//
//------------------------------------------------------------------------------

class uvm_tlm_analysis_fifo #(type T = int) extends uvm_tlm_fifo #(T);

  // Port: analysis_export #(T)
  //
  // The analysis_export provides the write method to all connected analysis
  // ports and parent exports:
  //
  //|  function void write (T t)
  //
  // Access via ports bound to this export is the normal mechanism for writing
  // to an analysis FIFO. 
  // See write method of <uvm_tlm_if_base #(T1,T2)> for more information.

  uvm_analysis_imp #(T, uvm_tlm_analysis_fifo #(T)) analysis_export;


  // Function: new
  //
  // This is the standard uvm_component constructor. ~name~ is the local name
  // of this component. The ~parent~ should be left unspecified when this
  // component is instantiated in statically elaborated constructs and must be
  // specified when this component is a child of another UVM component.

  function new(string name ,  uvm_component parent = null);
    super.new(name, parent, 0); // analysis fifo must be unbounded
    analysis_export = new("analysis_export", this);
  endfunction

  const static string type_name = "uvm_tlm_analysis_fifo #(T)";

  virtual function string get_type_name();
    return type_name;
  endfunction

  function void write(input T t);
    void'(this.try_put(t)); // unbounded => must succeed
  endfunction

endclass
