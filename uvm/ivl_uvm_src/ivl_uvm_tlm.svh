class ivl_uvm_tlm_base extends uvm_component;
    function new (string name = "ivl_uvm_tlm_base", uvm_component parent = null);
      super.new (name, parent);
    endfunction : new 
endclass : ivl_uvm_tlm_base


class ivl_uvm_tlm_pull_export extends ivl_uvm_tlm_base;
    function new (string name = "ivl_uvm_tlm_pull_port", uvm_component parent = null);
      super.new (name, parent);
    endfunction : new 
  
endclass : ivl_uvm_tlm_pull_export


class ivl_uvm_tlm_pull_port extends ivl_uvm_tlm_base;
    bit is_full;
    bit is_empty;
    int m_size;
    `IVL_UVM_MBX_T m_mbx_val;
  
    function new (string name = "ivl_uvm_tlm_pull_port", uvm_component parent = null);
      super.new (name, parent);
      is_full = 0;
      is_empty = 1;
    endfunction : new 
  
    virtual task put (input `IVL_UVM_MBX_T in_xn);
      //wait (is_full == 0);
      while (is_full == 1) begin
        #1;
      end
      m_mbx_val = in_xn;
      `g2u_printf (("%m Put: "))
      is_full = 1;
      is_empty = 0;
    endtask : put 
  
    virtual task get (output `IVL_UVM_MBX_T out_xn);
      //wait (is_empty == 0);
      while (is_empty == 1) begin
        #1;
      end
      out_xn = m_mbx_val;
      is_full = 0;
      is_empty = 1;
      `g2u_printf (("%m Get: "))
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
      `g2u_printf (("%m Peek: "))
    endtask : peek 
  
    virtual function int num ();
      return (is_full);
    endfunction : num 
  
    virtual function void connect (ivl_uvm_tlm_pull_export unused_tlm_exp);
    endfunction : connect
  
    virtual task get_next_item (output `IVL_UVM_MBX_T x0);
      `g2u_printf (("%m get_next_item: "))
      peek(x0);
    endtask : get_next_item
  
    virtual function void item_done ();
      bit rval;
      `IVL_UVM_MBX_T x0;
      rval = try_get(x0);
    endfunction : item_done
  
endclass : ivl_uvm_tlm_pull_port

