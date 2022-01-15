module main;

   reg enable, bar_a, bar_b, val_in;
   reg [7:0] scon;

   reg val;
   //(* ivl_synthesis_on *)
   always @(val_in or bar_a or bar_b or scon[7:6] or enable)
     begin
	if (scon[7:6]==2'b10) begin
	   val = 1'b1;
	end else if (enable) begin
	   val = val_in;
	end else begin
	   val = !bar_b & bar_a;
	end
     end

   (* ivl_synthesis_off *)
   initial begin
      val_in = 0;
      enable = 0;
      bar_b = 0;
      bar_a = 0;
      scon = 8'b10_000000;

      #1 if (val !== 1'b1) begin
	 $display("FAILED -- scon=%b, val=%b", scon, val);
	 $finish;
      end

      scon = 0;
      enable = 1;
      #1 if (val !== 1'b0) begin
	 $display("FAILED -- scon=%b, enable=%b, val=%b", scon, enable, val);
	 $finish;
      end

      val_in = 1;
      #1 if (val !== 1'b1) begin
	 $display("FAILED -- scon=%b, enable=%b, val_in=%b, val=%b",
		  scon, enable, val_in, val);
	 $finish;
      end

      enable = 0;
      #1 if (val !== 1'b0) begin
	 $display("FAILED -- scon=%b, enable=%b, val=%b", scon, enable, val);
	 $finish;
      end

      bar_a = 1;
      #1 if (val !== 1'b1) begin
	 $display("FAILED -- scon=%b, enable=%b, bar_a==%b, bar_b=%b, val=%b",
		  scon, enable, bar_a, bar_b, val);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
