/*
 * This is based on PR#1026.
 */
module main;

   reg [4:0] index;
   reg [31:0] foo;

   initial begin
      for (index = 0 ;  index < 31 ;  index = index + 1) begin
	 #1 $display("index=%d, foo=%b", index, foo);
      end
      $finish(0);
   end

   always @(*)
     begin
	foo = 32'b0;
	foo[index]=1'b1;
     end
endmodule
