// Check that positional assigment patterns are supported for structs when using
// continuous assignments.

module test;

  typedef struct packed {
    logic [31:0] x;
    logic [15:0] y;
    logic [7:0] z;
  } T;

  T x;

  // Check nested assignment patterns
  struct packed {
    T x;
    logic [2:0][3:0] y;
  } y;

  assign x = '{1'b1, 2.0, 2 + 1};
  assign y = '{'{1'b1, 2.0, 2 + 1}, '{4, 5, 6}};

  // Use an inital block with a delay since a final block cannot be converted to vlog95
  initial begin
    #1;
    if (x === 56'h00000001000203 &&
        y === 68'h00000001000203456) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
