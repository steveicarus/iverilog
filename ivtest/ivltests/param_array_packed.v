// Check that a multi-dimension packed array parameter can be initialized with
// an assignment pattern and that an index select returns the whole element,
// for both ascending and descending outer dimensions, and for both constant
// and variable indices. See GitHub issue #1180.

module test;

  // Ascending and descending outer dimension, same element values.
  localparam logic [0:7][3:0] PA = '{4'd8, 4'd9, 4'd10, 4'd11,
                                     4'd12, 4'd13, 4'd14, 4'd15};
  localparam logic [7:0][3:0] PD = '{4'd8, 4'd9, 4'd10, 4'd11,
                                     4'd12, 4'd13, 4'd14, 4'd15};

  // Reference nets initialized identically. Indexing these uses Icarus's
  // existing (independent) net path, so they define the expected behavior.
  logic [0:7][3:0] NA = '{4'd8, 4'd9, 4'd10, 4'd11,
                          4'd12, 4'd13, 4'd14, 4'd15};
  logic [7:0][3:0] ND = '{4'd8, 4'd9, 4'd10, 4'd11,
                          4'd12, 4'd13, 4'd14, 4'd15};

  // A constant element select must be usable in a constant expression.
  localparam int K = PA[2];

  integer i;
  integer errors = 0;

  initial begin
    for (i = 0 ; i < 8 ; i = i + 1) begin
      if (PA[i] !== NA[i]) begin
        errors = errors + 1;
        $display("FAILED: PA[%0d] = %0d, expected %0d", i, PA[i], NA[i]);
      end
      if (PD[i] !== ND[i]) begin
        errors = errors + 1;
        $display("FAILED: PD[%0d] = %0d, expected %0d", i, PD[i], ND[i]);
      end
    end

    if (K !== 10) begin
      errors = errors + 1;
      $display("FAILED: constant PA[2] = %0d, expected 10", K);
    end

    if (errors == 0)
      $display("PASSED");
  end

endmodule
