module top;
  reg pass = 1'b0;

  integer lp;
  reg [7:0] ival = 8'b0;

  initial begin
    /* If `ival' is replaced by a literal `0' this works... */
    for(lp = ival; lp < 5; lp = lp + 1) begin
      pass = 1'b1;
    end

    if(pass) $display("PASSED");
    else $display("FAILED");
  end
endmodule
