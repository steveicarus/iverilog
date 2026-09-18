// Check that a local class object hides an outer scope when calling a task
// method.

class C;

  task run;
    $display("PASSED");
  endtask

endclass

module object_type;

  task run;
    $display("FAILED");
  endtask

endmodule

module child;

  C object;

  initial begin
    object = new;
    object.run();
  end

endmodule

module test;

  object_type object();
  child i_child();

endmodule
