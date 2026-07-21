`define IVL_UVM_MBX_T apb_xn
// `define IVL_UVM_MBX_T int
`define uvm_info(ID, MSG, VERB)

package p;

  typedef enum {IDLE, APB_WR, APB_RD} apb_kind_e;

  class apb_xn;
    rand int addr;
    rand int data;
    rand apb_kind_e kind;

    function new ();
      addr = $urandom();
      data = $urandom();
    endfunction : new 

    virtual function void print ();
      $display ("%m addr: 0x%0h data: 0x%0h kind: %0d ",
	addr, data, kind);
    endfunction : print 
  endclass : apb_xn

  class ivl_uvm_mbx;
  endclass : ivl_uvm_mbx

  class uvm_phase;
  endclass : uvm_phase

  class ivl_uvm_tlm_export;
  endclass : ivl_uvm_tlm_export



  class ivl_uvm_tlm_drvr_port;
    bit is_full;
    bit is_empty;
    int m_size;
    `IVL_UVM_MBX_T m_mbx_val;
  
    function new (string name = "mailbox_int");
      //super.new (name);
      is_full = 0;
      is_empty = 1;
    endfunction : new 
  
    virtual task put (input `IVL_UVM_MBX_T in_xn);
      //wait (is_full == 0);
      while (is_full == 1) begin
        #1;
      end
      m_mbx_val = in_xn;
      //`g2u_printf (("MBX: Put: 0x%0h", in_xn))
      is_full = 1;
      is_empty = 0;
    endtask : put 
  
    virtual task get (output `IVL_UVM_MBX_T out_xn);
      //wait (is_empty == 0);
      while (is_empty == 1) begin
        #1;
      end
      out_xn = m_mbx_val;
      //`g2u_printf (("MBX: Get: 0x%0h", out_xn))
      is_full = 0;
      is_empty = 1;
    endtask : get 
  
    virtual function bit try_get (`IVL_UVM_MBX_T out_xn);
       bit rval;
       if (is_empty == 0) begin
         is_full = 0;
         is_empty = 1;
         out_xn = m_mbx_val;
         rval = 1;
       end
    endfunction : try_get 
  
  
    virtual task peek (output `IVL_UVM_MBX_T out_xn);
      //wait (is_empty == 0);
      while (is_empty == 1) begin
        #1;
      end
      out_xn = m_mbx_val;
      //`g2u_printf (("MBX: Peek: 0x%0h", out_xn))
    endtask : peek 
  
    virtual function int num ();
      return (is_full);
    endfunction : num 
  
    virtual function void connect (ivl_uvm_tlm_export unused_tlm_exp);
    endfunction : connect
  
    virtual task get_next_item (output `IVL_UVM_MBX_T x0);
      peek(x0);
    endtask : get_next_item
  
    virtual function void item_done ();
      bit rval;
      `IVL_UVM_MBX_T x0;
      rval = try_get(x0);
    endfunction : item_done
  
  endclass : ivl_uvm_tlm_drvr_port
endpackage : p

module ivl_uvm_driver;
  import p::*;
  `IVL_UVM_MBX_T req;
  `IVL_UVM_MBX_T rsp;

  ivl_uvm_tlm_drvr_port seq_item_port;
  
  initial begin
    seq_item_port = new ();
  end

  task run_phase (uvm_phase phase);

    forever begin
      seq_item_port.get_next_item (req);
      $display ("%m %0t Get:", $time);
      req.print();
      seq_item_port.item_done ();
    end
  endtask : run_phase

  uvm_phase u_ph_0;
  initial begin
    run_phase (u_ph_0);
  end

endmodule : ivl_uvm_driver

module ivl_uvm_sequencer;
  import p::*;
  ivl_uvm_tlm_export seq_item_export;

  initial begin
    seq_item_export = new ();
  end

endmodule : ivl_uvm_sequencer

module ivl_uvm_agent;
  import p::*;

  ivl_uvm_driver drvr ();
  ivl_uvm_sequencer sqr ();

  function void connect_phase (uvm_phase phase);
    drvr.seq_item_port.connect (sqr.seq_item_export);
  endfunction : connect_phase 

endmodule : ivl_uvm_agent


module ivl_uvm_sqr_drvr;
  import p::*;
  
  `IVL_UVM_MBX_T lv_put_val;
  `IVL_UVM_MBX_T lv_get_val;

  ivl_uvm_agent u_agent ();

  task do_puts();
    repeat (10) begin
      lv_put_val = new();
      u_agent.drvr.seq_item_port.put (lv_put_val);
      $display ("Test MBX: Put");
      lv_put_val.print();
      #10;
    end
  endtask : do_puts

   initial begin : test
     // run_test ();
     #100;
     `uvm_info("IVL_UVM", "UVM_MEDIUM: Hello World", UVM_MEDIUM) 

     fork
       do_puts();
     join
     #1000;

     $finish (2);
   end : test

endmodule : ivl_uvm_sqr_drvr

