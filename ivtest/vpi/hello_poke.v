module main;

   reg [5*8-1 : 0] hello;

   initial begin
      hello = "XXXXX";

      if (hello !== "XXXXX") begin
	 $display("FAILED -- X = %h", hello);
	 $finish;
      end

      $hello_poke(hello);

      if (hello !== "Hello") begin
	 $display("FAILED -- Hello = %h", hello);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
