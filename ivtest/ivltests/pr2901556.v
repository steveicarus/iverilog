module top;
  integer lp;

  wire signed [5:0] in = lp[5:0];

  // If these two are combined "$signed({1'b0,in[5]})" then this will work
  // as expected.
  wire [5:0] #1 base = (in + (in >>> 2)) >>> 2;
  wire signed [5:0] #1 fix = base + in[5];

//  wire [5:0] base; // If this is missing the program will core dump!
//  wire signed [5:0] #1 fix = ((in + (in >>> 2)) >>> 2) + $signed({1'b0,in[5]});

  wire [6:0] #1 res = in + fix;

  always @(*) $display("%0d: %d %d %d %d", $time, $signed(in), $signed(base),
                       $signed(fix), $signed(res));
  // It appears that the final calculation event is being lost for fix == -1.
  initial begin
    lp = -7;
    #5 lp = -5;
    #1 lp = -6;
    #5 if ($signed(res) !== -7) $display("FAILED");
    else $display("PASSED");
  end
endmodule
