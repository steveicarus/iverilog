package test_pkg;

  virtual class uvm_void;
  endclass : uvm_void

  class uvm_object extends uvm_void;
    function new (string name = "uvm_object");
       $display("uvm_object::new(%s)", name); // XXXX
       m_name = name;
    endfunction : new

    virtual function void print ();
      $display ("uvm_object::Print: m_name=%s", m_name);
    endfunction : print

     string m_name;

  endclass : uvm_object

  class uvm_report_object extends uvm_object;
    function new (string name = "uvm_report_object");
       // super.new must be the first statement in a constructor.
       // If it is not, generate an error.
       $display("uvm_report_object::new");
       super.new (name);
       $display("uvm_report_object::new");
    endfunction : new
  endclass : uvm_report_object

endpackage : test_pkg

module m;
   import test_pkg::*;
   uvm_object u0;
   uvm_report_object u1;

   initial begin : test
      #100;
      $display ("Hello World");
      u0 = new ();
      u0.print();
      u1 = new ();
      u1.print();

   end : test

endmodule : m
