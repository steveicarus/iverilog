module bts ( z , a , e);
  inout z ; wire z ;
  input a ; wire a ;
  input e ; wire e ;

  assign #4 z= ( (e==1'b1)? a : 1'bz );
endmodule

module test();
   reg [1:0] aa;
   wire [1:0] zz;
   reg [1:0]  ee;

   bts sub1 (.z(zz[1]), .a(aa[1]), .e(ee[1]));
   bts sub0 (.z(zz[0]), .a(aa[0]), .e(ee[0]));


  initial begin
//   $dumpvars;
   ee=2'b00;
   aa=2'b00; #100;
   if (zz !== 2'bzz) begin
      $display("FAILED -- (1) All disabled, expected HiZ, got %b", zz);
      $finish;
   end
   aa=2'b11; #100;
   if (zz !== 2'bzz) begin
      $display("FAILED -- (2) All disabled, expected HiZ, got %b", zz);
      $finish;
   end
   aa=2'b00; #100;
   if (zz !== 2'bzz) begin
      $display("FAILED -- (3) All disabled, expected HiZ, got %b", zz);
      $finish;
   end
   aa=2'b11; #100;
   if (zz !== 2'bzz) begin
      $display("FAILED -- (4) All disabled, expected HiZ, got %b", zz);
      $finish;
   end

   ee=2'b11;
   aa=2'b00; #100;
   if (zz !== 2'b00) begin
      $display("FAILED -- (5) All enabled, expected 00, got %b", zz);
      $finish;
   end
   aa=2'b11; #100;
   if (zz !== 2'b11) begin
      $display("FAILED -- (6) All enabled, expected 11, got %b", zz);
      $finish;
   end
   aa=2'b00; #100;
   if (zz !== 2'b00) begin
      $display("FAILED -- (7) All enabled, expected 00, got %b", zz);
      $finish;
   end
   aa=2'b11; #100;
   if (zz !== 2'b11) begin
      $display("FAILED -- (8) All enabled, expected 11, got %b", zz);
      $finish;
   end

   $display("PASSED");
  end
endmodule
