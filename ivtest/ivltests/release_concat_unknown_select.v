// Check that unknown bit selects do not skip other release targets in a concatenation.

module test;

  wire [3:0] value = 4'ha;
  wire hi = 1'b0;
  wire lo = 1'b1;

  initial begin
    force {hi, lo} = 2'b10;
    #1;
    release {hi, value[1'bx], lo, value[1'bz]};
    #1;

    if ({hi, lo, value} === 6'b01_1010) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
