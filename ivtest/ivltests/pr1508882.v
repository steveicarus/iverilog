/*
 * This test is based on pr1508882. The output from the test module
 * should produce a 5 bit result, and the widths of the vectors are
 * correct for that assumption. But if an implicit part select is
 * mitted in the assign out=tmp, then the vector widths can break.
 */
module main;

   reg [5:0]  a, b;
   wire [4:0] sum;

   test dut (.out(sum), .a(a), .b(b));

   wire [5:0] padded = {1'b0, sum};

   initial begin
      a = 1;
      b = 7;

      #1 if (padded !== (a+b)) begin
	 $display("FAILED -- sum=%0d, a=%0d, b=%0d", sum, a, b);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main

module test (output [4:0] out, input [5:0] a, b);

   wire [5:0] tmp = a + b;
   assign     out = tmp;

endmodule // test
