module main;

   wire [31:0] DB;
   reg	       E;

   X2 U (.DB(DB[31:8]), .E(E));
   Y1 V (.DB(DB[7:0]),  .E(E));

   initial begin
      E = 0;
      #1 if (DB !== 32'hzzzzzzzz) begin
	 $display("FAILED -- DB=%b", DB);
	 $finish;
      end

      E = 1;
      #1 if (DB !== 32'h9zzzzz87) begin
	 $display("FAILED -- DB=%b", DB);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main

module X2(inout wire [31:8] DB, input wire E);

   X1 uu (.DB(DB[31:28]), .E(E));

endmodule // X2

module X1(inout wire [31:28] DB, input wire E);

   wire foo = DB[31:28];
   assign DB[31:28] = E? 4'b1001 : 4'bzzzz;

endmodule // sub

module Y1(inout wire [7:0] DB, input wire E);

   wire foo = DB[7:0];
   assign DB[7:0] = E? 8'h87 : 8'hzz;

endmodule // sub
