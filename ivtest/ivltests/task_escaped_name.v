// Check that an escaped task name matching a hierarchical task name
// calls the correct task.

module M;
  task run;
    output [7:0] value;
    value = 42;
  endtask
endmodule

module test;
  M g();

  task \g.run ;
    output [7:0] value;
    value = 23;
  endtask

  reg [7:0] escaped, nested;

  initial begin
    \g.run (escaped);
    g.run(nested);

    if (escaped !== 23 || nested !== 42) $display("FAILED");
    else $display("PASSED");
  end
endmodule
