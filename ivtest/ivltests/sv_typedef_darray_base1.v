// Check that a vector base typeof a dynamic array type is evaluated in the
// scope where the array type is declared.

localparam A = 8;

typedef logic [A-1:0] T[];

module test;
  localparam A = 4;

  T x;

  initial begin
    x = new [1];
    x[0] = 8'hff;
    if (x[0] === 8'hff) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
