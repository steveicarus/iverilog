module test;

//  This example was adapted from:

// DRAFT STANDARD VERILOG HARDWARE DESCRIPTION LANGUAGE
// IEEE P1364-2005/D3, 1/7/04
// Section 4.4.2 "An example of an expression bit-length problem"
// pg. 59

reg [15:0] a, b, answer; // 16-bit regs

initial
  begin
    a = 16'h8000;
    b = 16'h8000;
    answer = (a + b + 0) >> 1; //will work correctly
    if ( answer != 16'h8000 )
      begin
          $display("FAILED -- expected 16'h8000  received 16'h%h", answer);
          $finish;
      end
    $display("PASSED");
  end

endmodule
