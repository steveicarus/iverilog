module top;
  parameter one  = 1'b1;
  parameter zero = 1'b0;

  // These should fail since a zero replication is invalid in this context.
  wire [3:0] ca_tru = one  ?   4'b0001 : {0{1'b0}};
  wire [3:0] ca_fal = zero ? {0{1'b0}} :   4'b0010;

  // We used to not check for this so just pass for that case
  initial $display("PASSED");
endmodule
