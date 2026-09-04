// $countones / $onehot / $countbits used inline in an expression at a width
// that is not a multiple of 32. Regresses the stale-high-bits bug in the
// VPI thread-vector getter. Existing sf_countones.v only assigns to a var.
module top;
  reg pass;
  reg [3:0]  an;
  reg [11:0] w12;

  initial begin
    pass = 1'b1;
    an   = 4'b1110;   // ~an = 0001 -> exactly one set bit
    w12  = 12'b0000_0010_0000;

    // The original failing idiom: one-cold / one-hot check inline.
    if (($countones(~an) == 1) !== 1'b1) begin
      $display("FAILED: $countones(~an)==1 should be true (an=%b)", an);
      pass = 1'b0;
    end

    if (($countones(~an) != 1) !== 1'b0) begin
      $display("FAILED: $countones(~an)!=1 should be false (an=%b)", an);
      pass = 1'b0;
    end

    // Inline in arithmetic context.
    if (($countones(~an) + 0) !== 1) begin
      $display("FAILED: $countones(~an)+0 should be 1, got %0d", $countones(~an) + 0);
      pass = 1'b0;
    end

    // Width that is not a multiple of 32, inline.
    if (($countones(w12) == 1) !== 1'b1) begin
      $display("FAILED: $countones(w12)==1 should be true (w12=%b)", w12);
      pass = 1'b0;
    end

    // Same value via a variable must of course still agree.
    if ($countones(~an) !== 1) begin
      $display("FAILED: $countones(~an) (self) should be 1");
      pass = 1'b0;
    end

    // Sibling functions share the same getter; check them inline too.
    if (($onehot(~an) == 1) !== 1'b1) begin
      $display("FAILED: $onehot(~an)==1 should be true");
      pass = 1'b0;
    end

    if (($countbits(an, 1'b1) == 3) !== 1'b1) begin
      $display("FAILED: $countbits(an,1)==3 should be true (an=%b)", an);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

endmodule
