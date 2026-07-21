// Mailbox via Q
//

module ivl_uvm_mbx #(parameter type T = int) ();

  int mbx_via_q [$];
  int q_size;
  T my_q[$];

  initial begin
    #1;
    $display ("Type: ", $typename(T));
  end

  task put (input int ival);
    mbx_via_q.push_back (ival);
    q_size = mbx_via_q.size();
  endtask : put 


  task get (output int oval);
    // IVL Bug 412 wait (mbx_via_q.size() > 0);
    wait (q_size > 0);
    $display ("MBX: Size: %0d", mbx_via_q.size());
    oval = mbx_via_q.pop_front ();
    q_size = mbx_via_q.size();
    $display ("MBX: Size: %0d", mbx_via_q.size());
  endtask : get 

endmodule : ivl_uvm_mbx 

module m;

  ivl_uvm_mbx mbx_0 ();
  ivl_uvm_mbx #(real) mbx_1 ();

  int pval,gval;

  initial begin
    pval = $random();
    mbx_0.put(pval);
    mbx_0.get(gval);
    $display ("put: 0x%0h get: 0x%0h", pval, gval);
  end
endmodule : m

