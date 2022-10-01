// Check that dynamic arrays with compatible packed base types can be passed as
// task arguments. Even it the element types are not identical.

module test;

  typedef logic [31:0] T[];

  task t1(logic [31:0] d[]);
    d[0] = 1;
  endtask

  task t2(logic [7:0][3:0] d[]);
    d[0] = 1;
  endtask

  task t3([31:0] d[]);
    d[0] = 1;
  endtask

  task t4(T d);
    d[0] = 1;
  endtask

  // For two packed types to be compatible they need to have the same packed
  // width, both be 2-state or 4-state and both be either signed or unsigned.
  logic [31:0] d1[];
  logic [7:0][3:0] d2[];

  initial begin
    d1 = new[1];
    d2 = new[1];
    t1(d1);
    t1(d2);
    t2(d1);
    t2(d2);
    t3(d1);
    t3(d2);
    t4(d1);
    t4(d2);

    $display("PASSED");
  end

endmodule
