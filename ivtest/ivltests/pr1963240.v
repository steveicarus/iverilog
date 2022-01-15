module test;
   reg [7:0] i8, j8;
   reg [31:0] i32;
   initial begin
      i8 = 32'hf7;
      j8 = 32'h3;
      i32 = $signed(i8 / j8);
      $write("expected %0h; got %0h\n", 8'h52, i32);
      i8 = 32'hf7;
      j8 = 32'h1;
      i32 = $signed(i8 / j8);
      $write("expected %0h; got %0h\n", 32'hffffff_f7, i32);
   end
endmodule
