/*
 * This is derived from PR#938 in the test suite.
 */
`timescale 1ns/100ps

module test;

   wire out;
   reg A, B, select;

   prim_mux2 mux (.in1(B), .in0(A), .select(select), .out(out));

   reg [3:0] cnt;
   initial begin
      $display("A B S     out");
      for (cnt = 0 ;  cnt <= 'b0111 ;  cnt = cnt + 1) begin
	 A <= cnt[0];
	 B <= cnt[1];
	 select <= cnt[2];

	 #1 $display("%b %b %b --> %b", A, B, select, out);
      end
   end

endmodule


primitive prim_mux2( output out, input in1, input in0, input select);

table

//in1 in0 select : out
    0   0      1 : 0;
    1   1      ? : 1;
    0   ?      1 : 0;
    1   ?      1 : 1;
    ?   0      0 : 0;
    ?   1      0 : 1;
endtable

endprimitive
