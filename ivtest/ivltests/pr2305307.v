`begin_keywords "1364-2005"
module top;
  reg passed, in, expect, out;
  integer lp;

  initial begin
    passed = 1'b1;

    for (lp=0; lp < 3 ; lp = lp + 1) begin
      case (lp)
        0: {in,expect} = 2'b00;
        1: {in,expect} = 2'b11;
        2: {in,expect} = 2'bzx;
        3: {in,expect} = 2'bxx;
      endcase

      // Check the normal reductions.
// These can fail be need a %buf opcode.
      out = &in;
      if (out !== expect) begin
        $display("FAILED reduction & with input %b, expected %b, got %b",
                 in, expect, out);
        passed = 1'b0;
      end

      out = |in;
      if (out !== expect) begin
        $display("FAILED reduction | with input %b, expected %b, got %b",
                 in, expect, out);
        passed = 1'b0;
      end

      out = ^in;
      if (out !== expect) begin
        $display("FAILED reduction ^ with input %b, expected %b, got %b",
                 in, expect, out);
        passed = 1'b0;
      end

      // Check the inverted reductions.
      out = ~&in;
      if (out !== ~expect) begin
        $display("FAILED reduction ~& with input %b, expected %b, got %b",
                 in, ~expect, out);
        passed = 1'b0;
      end

      out = ~|in;
      if (out !== ~expect) begin
        $display("FAILED reduction ~| with input %b, expected %b, got %b",
                 in, ~expect, out);
        passed = 1'b0;
      end

      out = ~^in;
      if (out !== ~expect) begin
        $display("FAILED reduction ~^ with input %b, expected %b, got %b",
                 in, ~expect, out);
        passed = 1'b0;
      end
    end

    if (passed) $display("PASSED");
  end
endmodule
`end_keywords
