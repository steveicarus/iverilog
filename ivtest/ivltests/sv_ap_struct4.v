// Check that positional assigment patterns are supported for structs when using
// continuous assignments to array elements.

module test;

  typedef struct packed {
    logic [31:0] x;
    logic [15:0] y;
    logic [7:0] z;
  } T;

  T x[2];

  // Check nested assignment patterns
  struct packed {
    T x;
    logic [2:0][3:0] y;
  } y[2];

  assign x[0] = '{1'b1, 2.0, 2 + 1};
  assign y[1] = '{'{1'b1, 2.0, 2 + 1}, '{4, 5, 6}};

  final begin
    if (x[0] === 56'h00000001000203 &&
        y[1] === 68'h00000001000203456) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
