package test_pkg;

  class uvm_object;
    function new ();
      print("new uvm_object");
    endfunction : new

    virtual function void print (string str);
      $display (str);
    endfunction : print
  endclass : uvm_object

  class uvm_report_object extends uvm_object;
    function new ();
      print("new uvm_report_object");
    endfunction : new
  endclass : uvm_report_object

endpackage : test_pkg

module m;
  import test_pkg::*;
  uvm_report_object r_0;
  uvm_object u_0;

   initial begin : test
     #100;
     u_0 = new();
     r_0 = new();

     u_0.print("u_0");
     r_0.print("r_0");

   end : test

endmodule : m
