
module test
  (output reg [4:0]  q,
   input wire [31:0] sel
   /* */);

   always @* begin
      casez (sel)
	32'b1zzz_zzzz_zzzz_zzzz__zzzz_zzzz_zzzz_zzzz: q = 5'd0;
	32'b01zz_zzzz_zzzz_zzzz__zzzz_zzzz_zzzz_zzzz: q = 5'd1;
	32'b001z_zzzz_zzzz_zzzz__zzzz_zzzz_zzzz_zzzz: q = 5'd2;
	32'b0001_zzzz_zzzz_zzzz__zzzz_zzzz_zzzz_zzzz: q = 5'd3;
	32'b0000_1zzz_zzzz_zzzz__zzzz_zzzz_zzzz_zzzz: q = 5'd4;
	32'b0000_01zz_zzzz_zzzz__zzzz_zzzz_zzzz_zzzz: q = 5'd5;
	32'b0000_001z_zzzz_zzzz__zzzz_zzzz_zzzz_zzzz: q = 5'd6;
	32'b0000_0001_zzzz_zzzz__zzzz_zzzz_zzzz_zzzz: q = 5'd7;

	32'b0000_0000_1zzz_zzzz__zzzz_zzzz_zzzz_zzzz: q = 5'd8;
	32'b0000_0000_01zz_zzzz__zzzz_zzzz_zzzz_zzzz: q = 5'd9;
	32'b0000_0000_001z_zzzz__zzzz_zzzz_zzzz_zzzz: q = 5'd10;
	32'b0000_0000_0001_zzzz__zzzz_zzzz_zzzz_zzzz: q = 5'd11;
	32'b0000_0000_0000_1zzz__zzzz_zzzz_zzzz_zzzz: q = 5'd12;
	32'b0000_0000_0000_01zz__zzzz_zzzz_zzzz_zzzz: q = 5'd13;
	32'b0000_0000_0000_001z__zzzz_zzzz_zzzz_zzzz: q = 5'd14;
	32'b0000_0000_0000_0001__zzzz_zzzz_zzzz_zzzz: q = 5'd15;

	32'b0000_0000_0000_0000__1zzz_zzzz_zzzz_zzzz: q = 5'd16;
	32'b0000_0000_0000_0000__01zz_zzzz_zzzz_zzzz: q = 5'd17;
	32'b0000_0000_0000_0000__001z_zzzz_zzzz_zzzz: q = 5'd18;
	32'b0000_0000_0000_0000__0001_zzzz_zzzz_zzzz: q = 5'd19;
	32'b0000_0000_0000_0000__0000_1zzz_zzzz_zzzz: q = 5'd20;
	32'b0000_0000_0000_0000__0000_01zz_zzzz_zzzz: q = 5'd21;
	32'b0000_0000_0000_0000__0000_001z_zzzz_zzzz: q = 5'd22;
	32'b0000_0000_0000_0000__0000_0001_zzzz_zzzz: q = 5'd23;

	32'b0000_0000_0000_0000__0000_0000_1zzz_zzzz: q = 5'd24;
	32'b0000_0000_0000_0000__0000_0000_01zz_zzzz: q = 5'd25;
	32'b0000_0000_0000_0000__0000_0000_001z_zzzz: q = 5'd26;
	32'b0000_0000_0000_0000__0000_0000_0001_zzzz: q = 5'd27;
	32'b0000_0000_0000_0000__0000_0000_0000_1zzz: q = 5'd28;
	32'b0000_0000_0000_0000__0000_0000_0000_01zz: q = 5'd29;
	32'b0000_0000_0000_0000__0000_0000_0000_001z: q = 5'd30;
	32'b0000_0000_0000_0000__0000_0000_0000_0001: q = 5'd31;
	default: q = 5'd0;
      endcase
   end // always @ *

endmodule // test

module main;
   reg [31:0] sel;
   wire [4:0] q;

   test dut (.q(q), .sel(sel));

   integer    idx;
   integer    rept;
   reg [31:0] mask, setb;
   initial begin
      sel = 0;
      #1 if (q !== 5'd0) begin
	 $display("FAILED -- sel=%b, q=%b", sel, q);
	 $finish;
      end

      for (idx = 0 ; idx < 32 ; idx = idx+1) begin
	 mask = 32'h7fff_ffff >> idx;
	 setb = mask + 32'd1;
	 for (rept = 0 ; rept < 4 ; rept = rept+1) begin
	    sel = setb | (mask & $random);
	    #1 if (q !== idx[4:0]) begin
	       $display("FAILED -- sel=%b, q=%b, idx=%0d", sel, q, idx);
	       $finish;
	    end
	 end
      end

      $display("PASSED");
   end
endmodule
