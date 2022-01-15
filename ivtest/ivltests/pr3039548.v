module check_this;

reg [5:0] offset;
reg [9:0] enablemask;
   initial begin
     enablemask = 10'b00000_00110;
     offset     = 0;
     $display("%b", {enablemask, (16'h0 +  8'h80 + offset )});
     $display("%b", {enablemask, (16'h0 + (8'h80 + offset))});
   end
endmodule // check_this
