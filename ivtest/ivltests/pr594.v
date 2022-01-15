module test;
   parameter NBytes = 21;
   parameter  Message = "0H1d2j3i4k 5R6i7k8d9["; // Message to send

   integer   J;
   reg [7:0] RSData;
   initial begin
      for (J=(NBytes-1); J>=0; J=J-1)
	begin
	   RSData = (Message>>(J*8)) & 8'hFF;
	   $display("RSData=%h", RSData);
	end
   end
endmodule // test
