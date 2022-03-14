// Check that it is possible to declare the data type for a packed array type
// task port separately from the direction for non-ANSI style port declarations.

module test;

  typedef logic [7:0] T1;
  typedef T1 [3:0] T2;

  task t;
    input x;
    T2 x;
    if (x[0] == 1 && x[1] == 2 && x[2] == 3 && x[3] == 4 &&
        $bits(x) == $bits(T2)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial begin
    static T2 val;
    val[0] = 8'h1;
    val[1] = 8'h2;
    val[2] = 8'h3;
    val[3] = 8'h4;
    t(val);
  end

endmodule
