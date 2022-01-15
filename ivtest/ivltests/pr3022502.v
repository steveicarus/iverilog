/* Verify that a tail recursive real ternary expression does not
 * overflow the available thread words. */
module top;
  reg pass;
  real vout;
  integer j;

  always @(j) begin
    vout = (j ==  0) ? 0.0 :
           (j ==  1) ? 0.1 :
           (j ==  2) ? 0.2 :
           (j ==  3) ? 0.3 :
           (j ==  4) ? 0.4 :
           (j ==  5) ? 0.5 :
           (j ==  6) ? 0.6 :
           (j ==  7) ? 0.7 :
           (j ==  8) ? 0.8 :
           (j ==  9) ? 0.9 :
           (j == 10) ? 1.0 :
           (j == 11) ? 1.1 :
           (j == 12) ? 1.2 :
           (j == 13) ? 1.3 :
           (j == 14) ? 1.4 :
           (j == 15) ? 1.5 :
           (j == 16) ? 1.6 :
           (j == 17) ? 1.7 :
           (j == 18) ? 1.8 :
           (j == 19) ? 1.9 : 0.0;
  end

  initial begin
    pass = 1'b1;

    for (j=0; j<20; j=j+1) begin
      #1;
      if (vout != j/10.0) begin
        $display("Failed: at %0d, got %f", j, vout);
        pass = 1'b0;
      end
    end

    if (pass) $display("PASSED");
  end
endmodule
