module top;
  real rl1, rl2;
  wire eq, ne, gt, ge, lt, le;
  reg passed = 1'b1;

  // Check that a decimal constant is converted to a real value.
  assign eq = rl2 == 0;
//  assign eq = rl2 == rl1;
  assign ne = rl2 != rl1;
  assign gt = rl2 > rl1;
  assign ge = rl2 >= rl1;
  assign lt = rl2 < rl1;
  assign le = rl2 <= rl1;

  initial begin
    rl1 = 0.0;
    rl2 = 0.0;
    #1 if ({eq,ne,gt,ge,lt,le} != 6'b100101) begin
      $display("Failed: expected %b, received %b", 6'b100101,
               {eq,ne,gt,ge,lt,le});
      passed = 1'b0;
    end
    #1 rl2 = -1.0;
    #1 if ({eq,ne,gt,ge,lt,le} != 6'b010011) begin
      $display("Failed: expected %b, received %b", 6'b010011,
               {eq,ne,gt,ge,lt,le});
      passed = 1'b0;
    end
    #1 rl2 = 1.0;
    #1 if ({eq,ne,gt,ge,lt,le} != 6'b011100) begin
      $display("Failed: expected %b, received %b", 6'b001100,
               {eq,ne,gt,ge,lt,le});
      passed = 1'b0;
    end
    if (passed) $display("PASSED");
  end
endmodule
