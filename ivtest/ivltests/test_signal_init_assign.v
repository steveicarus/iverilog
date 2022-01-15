module main;

   reg src;
   reg clk;
   wire dst0, dst1;

test #(.parm(0)) test0 (.dst(dst0), .src(src), .clk(clk));
test #(.parm(1)) test1 (.dst(dst1), .src(src), .clk(clk));

//Note: For Modelsim compatibility do instantiation as:
//test #(.parm(2'b10)) test0 (.dst(dst0), .src(src), .clk(clk));
//test #(.parm(2'b11)) test1 (.dst(dst1), .src(src), .clk(clk));
//The reason is that Modelsim handles single-bit std_logic as an
//enumeration, and enumeration values 2 and 3 correspond to the
//stdlogic '0' and '1' values. The integer to std_logic values
//in modelsim are:
   //  0   - 'U'
   //  1   - 'X'
   //  2   - '0'
   //  3   - '1'
   //  4   - 'Z'
   //  5   - 'W'
   //  6   - 'L'
   //  7   - 'H'
   //  8   - '-'
//Maybe in the future we'll have to do something similar?


   initial begin
      clk = 0;
      src = 0;
      #1 clk = 1;
      #1 if (dst0 !== 1'b0 || dst1 !== 1'b1) begin
	 $display("FAILED: src=%b, dst0=%b dst1=%b", src, dst0, dst1);
	 $finish;
      end
      clk = 0;
      src = 1;
      #1 clk = 1;
      #1 if (dst0 !== 1'b1 || dst1 !== 1'b0) begin
	 $display("FAILED: src=%b, dst0=%b dst1=%b", src, dst0, dst1);
	 $finish;
      end
      $display("PASSED");
   end // initial begin




endmodule // main
