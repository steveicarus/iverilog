module test();
   parameter N_CH = 1;

   reg pass = 1'b1;
   reg clk;
   reg [31:0] data[0: N_CH-1];

   generate
      genvar  i;
      for (i=0; i<N_CH; i = i + 1)
         begin: sdac
            always @ (posedge clk)
               data[i] <= 32'h0;
         end
   endgenerate

   initial
      begin
         clk = 0;
         if (data[0] != 32'bx) begin
            $display("FAILED: initial value, expected 32'bx, got %b", data[0]);
            pass = 1'b0;
         end
         #26 // Check after the first clock edge.
         if (data[0] != 32'b0) begin
            $display("FAILED: final value, expected 32'b0, got %b", data[0]);
            pass = 1'b0;
         end
         if (pass) $display("PASSED");
         $finish;
      end

   always #25 clk = ~clk;
endmodule // test
