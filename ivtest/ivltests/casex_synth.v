/*
 */
module main;

   reg [1:0] sel, in;
   reg [1:0] out;

   (* ivl_combinational *)
   always @*
     casex (sel)
       2'b0?: out = 2'b10;
       2'b10: out = in[0];
       2'b11: out = in[1];
     endcase // casex(sel)

   (* ivl_synthesis_off *)
   initial begin
      in = 2'b10;

      sel = 0;
      #1 if (out !== 2'b10) begin
	 $display("FAILED -- sel=%b, out=%b", sel, out);
	 $finish;
      end

      sel = 1;
      #1 if (out !== 2'b10) begin
	 $display("FAILED -- sel=%b, out=%b", sel, out);
	 $finish;
      end

      sel = 2;
      #1 if (out !== 2'b00) begin
	 $display("FAILED -- sel=%b, in=%b, out=%b", sel, in, out);
	 $finish;
      end

      sel = 3;
      #1 if (out !== 2'b01) begin
	 $display("FAILED -- sel=%b, in=%b, out=%b", sel, in, out);
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
