// Check that positional assigment patterns are supported for structs are
// supported for parameters.

module test;

  typedef struct packed {
    int x;
    shortint y;
    byte z;
  } T;

  localparam T x = '{1'b1, 2.0, 2 + 1};

  initial begin
    if (x === 56'h00000001000203) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
