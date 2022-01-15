// (c) 2001 Kenji KISE ivl-bugs PR#307

module top;
   reg [63:0] in1,in2;
   reg [63:0] out;

   initial begin
      in1 = 64'hffffffffffffffff;
      in2 = 64'hfffffffffffffff7;
      out = in1 + in2;
      $display("%h + %h =  %h", in1,in2,out);
      if (out === 64'hfffffffffffffff6)
	$display("PASSED");
      else
	$display("FAILED");
   end
endmodule
