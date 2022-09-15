// Check that it is an error to declare a non-ANSI task port with implicit
// packed dimensions if it is later redeclared as a vector typed variable and
// the size of the packed dimensions do not match.

module test;

  task t;
    input [7:0] x;
    reg [3:0] x;
    $display("FAILED");
  endtask

  initial begin
    t(10);
  end

endmodule
