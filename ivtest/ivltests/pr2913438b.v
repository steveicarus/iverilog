module top;
  reg [6:0] ltl;
  reg signed [6:0] ltl_s;
  reg [15:0] big;
  reg result, pass;

  initial begin
    pass = 1'b1;

    // An unsigned value should be zero padded.
    ltl = 7'd127;
    result = test(ltl);
    if (result) begin
      $display("Failed: unsigned argument was sign extended");
      pass = 1'b0;
    end

    // This should be evaluated in an eight bit context since the
    // function argument is eight bits. This will set the eight bit.
    result = test(ltl+7'd1);
    if (!result) begin
      $display("Failed: function width does not determines expression width.");
      pass = 1'b0;
    end

    // A signed value should be sign padded.
    ltl_s = -7'd1;
    result = test(ltl_s);
    if (!result) begin
      $display("Failed: signed argument was not sign extended");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

  function test ;
    input signed [7:0] in;
    begin
      if (in[7]) test = 1'b1;
      else test = 1'b0;
    end
  endfunction
endmodule
