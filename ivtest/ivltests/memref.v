/*
 * Check simple scope up-reference of memories.
 */
module main;

   reg [7:0] foo [0:5];

   integer   idx;
   task showstring;
      begin
	 for (idx = 0 ; idx < 6 ; idx = idx+1) begin
	    $write("%c", foo[idx]);
	 end
	 $display;
      end
   endtask // showstring

   initial begin
      foo[0] = "P";
      foo[1] = "A";
      foo[2] = "S";
      foo[3] = "S";
      foo[4] = "E";
      foo[5] = "D";
      showstring;
   end
endmodule // main
