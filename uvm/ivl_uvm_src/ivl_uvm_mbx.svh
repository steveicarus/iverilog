
`ifndef IVL_UVM_MBX_T 
  `define IVL_UVM_MBX_T int
`endif

// Poor man's mailbox
// Default size = 1 in SV
// Here, only size-1 is supported as of now
// Also no parameterized classes in Icarus (yet), so using a `define
//
// Only one type of mailbox is supported as of now
//
class ivl_uvm_mbx extends uvm_object;
  bit is_full;
  bit is_empty;
  int m_size;
  `IVL_UVM_MBX_T m_mbx_val;

  function new (string name = "mailbox_int");
    super.new (name);
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


endclass : ivl_uvm_mbx

