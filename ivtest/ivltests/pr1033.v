/*
 * Test case showing the failure of the 'less than or equal' operator (note
 * that the 'greather than or equal' comparison also fails) on two signed
 * values.  The 'foo' module defines inputs 'a' and 'b' as signed inputs,
 * then performs a LTE comparison of those two values in order to select the
 * smaller of the two as the result (via a mux).  The generated output for
 * this test (via the display call) for icarus and a well known commercial
 * Verilog simulator are shown here.  It is my belief that the commercial
 * simulator results reflect the correct behavior for a simulator.
 * Specifically, with signed numbers the value 32'h7fffffff represents the
 * maximum positive value while 32'h80000000 represents the minimum negative
 * value.  Thus for Less Than or Equal comparison any negative value (ie
 * 32'h80000000) should evaluate to less than any positive value
 * (ie 32'h7fffffff).  Note the difference in the last 4 comparisons between
 * the icarus results and the commercial results.  The commercial results show
 * that the 32'h8000000? values are less than the 32'h7ffffff? values as is
 * expected.
 *
 *              icarus                    commercial simulator
 *   7ffffff5 7ffffffa = 7ffffff5    # 7ffffff5 7ffffffa = 7ffffff5
 *   7ffffff6 7ffffffb = 7ffffff6    # 7ffffff6 7ffffffb = 7ffffff6
 *   7ffffff7 7ffffffc = 7ffffff7    # 7ffffff7 7ffffffc = 7ffffff7
 *   7ffffff8 7ffffffd = 7ffffff8    # 7ffffff8 7ffffffd = 7ffffff8
 *   7ffffff9 7ffffffe = 7ffffff9    # 7ffffff9 7ffffffe = 7ffffff9
 *   7ffffffa 7fffffff = 7ffffffa    # 7ffffffa 7fffffff = 7ffffffa
 *   7ffffffb 80000000 = 7ffffffb    # 7ffffffb 80000000 = 80000000
 *   7ffffffc 80000001 = 7ffffffc    # 7ffffffc 80000001 = 80000001
 *   7ffffffd 80000002 = 7ffffffd    # 7ffffffd 80000002 = 80000002
 *   7ffffffe 80000003 = 7ffffffe    # 7ffffffe 80000003 = 80000003
 *
 * iverilog -version:
 *     Icarus Verilog version 0.7 ($Name:  $)
 *
 * Compilation
 *    iverilog -o iverilog.out
 *    vvp iverilog.out
 */
module test ();
   reg clk;
   reg [31:0] a_dat;
   reg [31:0] b_dat;
   wire [31:0] result;

   initial begin
	  clk <= 0;
	  a_dat <= 32'h7fffFFF5;
	  b_dat <= 32'h7fffFFFA;
	  #500 $finish(0);
   end
   always #25 clk <= ~clk;

   always @(posedge clk)
	 begin
		a_dat <= a_dat + 1;
		b_dat <= b_dat + 1;
		$display("%x %x = %x", a_dat, b_dat, result);
	 end

   foo foo_test(.a(a_dat), .b(b_dat), .RESULT(result));

endmodule // test

module foo(a, b, RESULT);
   input	signed [31:0]	a;
   input	signed [31:0]	b;
   output [31:0] RESULT;
   wire			 lessThanEqualTo;
   wire [31:0]		 mux;
   assign		 lessThanEqualTo=a<=b;
   assign		 mux=(lessThanEqualTo)?a:b;
   assign		 RESULT=mux;
endmodule // foo
