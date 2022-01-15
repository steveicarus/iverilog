//
// The output from the display should be:
//   i is '1'; j is '111'; k is '0'
//
module test;
   reg one = 1;
   reg i, k, kr;
   reg [2:0] j, jr;
   initial
     begin
	i = ($signed(3'b111) === 3'b111);
	j = $signed(3'b110) >>> 1;
	jr = $signed(3'b110) >>> one;
	k = (($signed(3'b110) >>> 1) === 3'b111);
	kr = (($signed(3'b110) >>> one) === 3'b111);
	$display("i is '%b'; j is '%b'; k is '%b'", i, j, k);
	$display("runtime ; j is '%b'; k is '%b'", jr, kr);
     end
endmodule
