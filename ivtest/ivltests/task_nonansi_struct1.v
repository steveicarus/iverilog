// Check that it is possible to declare the data type for a struct type task
// port separately from the direction for non-ANSI style port declarations.

module test;

  typedef struct packed {
    reg [31:0] x;
    reg [7:0] y;
  } T;

  task t;
    input x;
    T x;
    if (x.x == 10 && x.y == 20 && $bits(x) == $bits(T)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial begin
    static T val;
    val.x = 10;
    val.y = 20;
    t(val);
  end

endmodule
