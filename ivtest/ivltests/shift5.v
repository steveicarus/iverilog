/*
 * This example pulled from comp.lang.verilog. It was written
 * by Russell Fredrickson <russell_fredrickson@hp.com> to test
 * arithmetic shift for other compilers, and caught mine.
 */

module ArithmeticShiftTest;
  reg signed [31:0] in;
  reg        [5:0]  shift;
  reg signed [31:0] out;

  //calculate arithmetic barrel shift right
  always@(*) out = in >>> shift;

  initial begin
    //set up inputs for always block
    in = 32'sh80000000;//set to highest value negative number(-2147483648)
    shift = 6'd32;     //shift the entire width of the word

    #1; //allow time for inputs to propagate

    //check output
    if(out === (32'sh80000000 >>> 6'd32)) begin
      $display("PASS: 32'sh80000000 >>> 6'd32 = 0x%h", out);
    end
    else begin
      $display("FAIL: 32'sh80000000 >>> 6'd32 != 0x%h,",
        (32'sh80000000 >>> 6'd32), " actual = 0x%h.", out);
    end
  end // initial begin
endmodule // ArithmeticShiftTest
