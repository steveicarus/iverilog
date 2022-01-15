module main;

   reg [2:0] ADDR;
   wire [1:0] data0 = 0, data1 = 1, data2 = 2, data3 = 3;

   reg [1:0]  data;
   always @*
     case (ADDR[2:0])
       3'b000: data = data0;
       3'b001: data = data1;
       3'b010: data = data2;
       3'b011: data = data3;
       default: data = 0;
     endcase // case(ADDR[2:0])

   initial begin
      ADDR = 0;
      #1 $display("data=%b", data);
      if (data !== ADDR) begin
	 $display("FAILED");
	 $finish;
      end

      ADDR = 1;
      #1 $display("data=%b", data);
      if (data !== ADDR) begin
	 $display("FAILED");
	 $finish;
      end

      ADDR = 2;
      #1 $display("data=%b", data);
      if (data !== ADDR) begin
	 $display("FAILED");
	 $finish;
      end

      ADDR = 3;
      #1 $display("data=%b", data);
      if (data !== ADDR) begin
	 $display("FAILED");
	 $finish;
      end

      ADDR = 4;
      #1 $display("data=%b", data);
      if (data !== 0)begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
