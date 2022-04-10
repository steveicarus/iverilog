// Check that a vector base typeof a queue type is evaluated in the scope where
// the array type is declared.

localparam A = 8;

typedef logic [A-1:0] T[$];

module test;
  localparam A = 4;

  T x;

  initial begin
    x.push_back(8'hff);
    if (x[0] === 8'hff) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
