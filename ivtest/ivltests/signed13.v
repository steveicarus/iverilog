`begin_keywords "1364-2005"
module top;
  reg pass;
  reg [3:0] var;
  integer lp;

  initial begin
    pass = 1'b1;
    for (lp = 0; lp < 16; lp = lp + 1) begin
      var = lp;
      // This should bit extend var as unsigned and then
      // convert it into a signed value.
      if (lp !== $signed(var+5'b0)) begin
        $display("FAILED: expected %2d, got %2d", lp, $signed(var+5'b0));
        pass = 1'b0;
      end
    end

    if (pass) $display("PASSED");
  end
endmodule
`end_keywords
