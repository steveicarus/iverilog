/*
 * Notice how the port direction and type are declared
 * together in each statement.
 */
module one_a(sum,co,a,b,ci);
  output reg sum;
  output reg co;
  input wire a;
  input wire b;
  input wire ci;

always@(a or b or ci)
  begin
    sum = a ^ b ^ ci;
    co  = a*b || a*ci || b*ci;
  end
endmodule


module main;

   wire sum, co;
   reg [3:0] in;

   one_a dut (sum, co, in[0], in[1], in[2]);

   initial begin
      in = 0;
      #1 for (in = 0 ;  in[3] == 0 ;  in = in + 1) begin
	 #1 $display("in=%b; co/sum = %b/%b", in, co, sum);
      end
   end

endmodule // main
