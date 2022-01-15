`begin_keywords "1364-2005"
module top;
  reg pass;
  real rvar;
  wire [3:0] var;

  assign var = rvar;

  initial begin
    pass = 1'b1;
    rvar <= 1'b0;
    #1 rvar = 1'b1;
    #1 rvar = 2'b10;
    #1 rvar = 2'b11;
    #1 if (pass) $display("PASSED");
  end

  real_to_bit u1(rvar);
  real_to_real u2(rvar);
  real_to_real u3[1:0](rvar);
  real_to_vec u4(rvar);
  real_to_vec u5[1:0](rvar);
  bit_to_real u6(var[0]);
  vec_to_real u7(var);
  vec_to_real u8[1:0](var);
endmodule

// Check a real value going to a single bit.
module real_to_bit (input wire in);
  always @(in) if (in !== $stime%2) begin
    $display("Failed real_to_bit %m at %1d, got %b, expected %2b",
             $stime, in, $stime%2);
    top.pass = 1'b0;
  end
endmodule

// Check a real value going to a real wire.
module real_to_real (input wire real in);
  always @(in) if (in != $stime) begin
    $display("Failed real_to_real %m at %1d, got %0d, expected %0d",
             $stime, in, $stime);
    top.pass = 1'b0;
  end
endmodule

// Check a real value going to multiple bit.
module real_to_vec (input wire [3:0] in);
  always @(in) if (in !== $stime) begin
    $display("Failed real_to_vec %m at %1d, got %0d, expected %0d",
             $stime, in, $stime);
    top.pass = 1'b0;
  end
endmodule

// Check a single bit going to a real wire.
module bit_to_real (input wire real in);
  always @(in) if (in != $stime%2) begin
    $display("Failed bit_to_real %m at %1d, got %0d, expected %0d",
             $stime, in, $stime%2);
    top.pass = 1'b0;
  end
endmodule

// Check a vector going to a real wire.
module vec_to_real (input wire real in);
  always @(in) if (in != $stime) begin
    $display("Failed vec_to_real %m at %1d, got %0d, expected %0d",
             $stime, in, $stime);
    top.pass = 1'b0;
  end
endmodule
`end_keywords
