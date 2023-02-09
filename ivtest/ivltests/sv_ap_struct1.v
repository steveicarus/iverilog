// Check that positional assigment patterns are supported for structs.

module test;

  typedef struct packed {
    int x;
    shortint y;
    byte z;
  } T;

  T x = '{1'b1, 2.0, 2 + 1};

  // Check nested assignment patterns
  struct packed {
    T x;
    bit [2:0][3:0] y;
  } y = '{'{1'b1, 2.0, 2 + 1}, '{4, 5, 6}};

  initial begin
    if (x === 56'h00000001000203 &&
        y === 68'h00000001000203456) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
