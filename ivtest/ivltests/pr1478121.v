`begin_keywords "1364-2005"
module main;

   wire [1:0] di;
   reg [1:0]  do;
   reg	      dir;

   wire [1:0] q;

   sub dut(.Q({q[0],q[1]}), .Di(di), .Do(do), .dir(dir));

   initial begin
      dir = 0;
      do = 2'b10;

      #1 if (q !== 2'bzz) begin
	 $display("FAILED -- q=%b, dir=%b", q, dir);
	 $finish;
      end

      dir = 1;

      #1 if (q !== 2'b01) begin
	 $display("FAILED -- q=%b, dir=%b, do=%b", q, dir, do);
	 $finish;
      end

      if (di !== 2'b10) begin
	 $display("FAILED -- di=%b, dir=%b, do=%b", di, dir, do);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main

module sub(inout [1:0]Q,
	   output[1:0]Di,
	   input [1:0]Do,
	   input dir);

   assign	 Di = Q;
   assign	 Q = dir? Do : 2'bzz;
endmodule // sub
`end_keywords
