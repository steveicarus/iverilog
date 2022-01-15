// pr1664684

module bug (rdo, rm, cpen, up14, rdi);
   output [31:0] rdo;
   input	 rm, cpen;
   input [31:0]  up14, rdi;

   initial $monitor($time,,rdo,,rm,cpen,,up14,,rdi);
   assign	 rdo = (rm | cpen) ? up14 : rdi;

 endmodule

 module bench;

   reg [31:0] up14;
   wire [31:0] rdo;
   reg	       rm, cpen;
   tri0 [31:0] rdi;

   bug u1 (rdo, rm, cpen, up14, rdi);

   initial begin
      rm = 1'bX;
      cpen = 1'b0;
      up14 = 'hX;
      #40;
      up14 = 32'd0;
      rm = 1'b0;
      #40;
      $finish(0);
   end
 endmodule
