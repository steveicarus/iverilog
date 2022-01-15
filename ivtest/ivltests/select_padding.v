module top;
  reg pass;
  reg [7:0] in;
  reg [1:0] res;
  integer lp;

  initial begin
    pass = 1'b1;
    in = 8'b11100100;
    lp = 3;

    // lp[1:0] is being sign extended and that fails when the value mod 4
    // is either 2 or 3! A bit/part select is always unsigned unless we use
    // The $signed function to cast it to signed!
    res = in[lp[1:0]*2 +: 2];
    if (res !== 2'b11) begin
      $display("Failed expected 2'b11, got %b (%b:%d)", res, in, lp[1:0]*2);
      pass = 1'b0;
    end

    // This should give -2 for the index.
    res = in[$signed(lp[1:0])*2 +: 2];
    if (res !== 2'bxx) begin
      $display("Failed expected 2'bxx, got %b (%b:%d)", res, in,
               $signed(lp[1:0])*2);
      pass = 1'b0;
    end

    lp = 6;
    // The same as above, but not at the start of the signal.
    res = in[lp[2:1]*2 +: 2];
    if (res !== 2'b11) begin
      $display("Failed expected 2'b11, got %b (%b:%d)", res, in, lp[2:1]*2);
      pass = 1'b0;
    end
    res = in[$signed(lp[2:1])*2 +: 2];
    if (res !== 2'bxx) begin
      $display("Failed expected 2'bxx, got %b (%b:%d)", res, in,
               $signed(lp[2:1])*2);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
