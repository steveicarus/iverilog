module tb;
parameter P = '1;
parameter W = 82;
wire [W-1:0] one = '1;
wire [W-1:0] zero = '0;
wire [W-1:0] x = 'x;
wire [W-1:0] z = 'z;
wire [15:0] expr_add = 16'h0 + '1;
wire [15:0] expr_xor = 16'haaaa ^ '1;
wire [3:0] bitsel = 4'h2;
wire [3:0] bitsel_lv;
wire [3:0] param = P;

assign bitsel_lv['1] = 1'b1;

initial begin
   #5;
   if (4'hf !== '1) begin
      $display("FAILED, 4'hf !== '1");
      $finish;
   end

   if (one !== '1) begin
      $display("FAILED, one !== '1");
      $finish;
   end

   if (one !== {W{1'b1}}) begin
      $display("FAILED, one = %b", one);
      $finish;
   end

   if (zero !== {W{1'b0}}) begin
      $display("FAILED, zero = %b", zero);
      $finish;
   end

   if (x !== {W{1'bx}}) begin
      $display("FAILED, x = %b", x);
      $finish;
   end

   if (z !== {W{1'bz}}) begin
      $display("FAILED, z = %b", z);
      $finish;
   end

   if (expr_add !== 16'hffff) begin
      $display("FAILED, expr_add = %b", expr_add);
      $finish;
   end

   if (expr_xor !== 16'h5555) begin
      $display("FAILED, expr_xor = %b", expr_xor);
      $finish;
   end

   if (bitsel_lv[1] !== 1'b1) begin
      $display("FAILED, bitsel_lv[1] = %b", bitsel_lv[1]);
      $finish;
   end

   if (bitsel['1] !== 1'b1) begin
      $display("FAILED, bitsel['1] = %b", bitsel['1]);
      $finish;
   end

   if (bitsel['1:'0] !== 2'b10) begin
      $display("FAILED, bitsel['1:'0] = %b", bitsel['1:'0]);
      $finish;
   end

   if (param !== 4'h1) begin
      $display("FAILED, param = %b, %b", param, P);
      $finish;
   end

   $display("PASSED");
end

endmodule
