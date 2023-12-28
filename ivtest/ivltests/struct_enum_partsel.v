module main;

   typedef enum logic [2:0] {
      ENUM_VAL = 3'b110
   } enumtype;

   typedef struct packed {
      enumtype e;
   } structtype;

   structtype s;

   initial
   begin
      s.e = ENUM_VAL;
      $display("s.e[2] = %d, s.e[1] = %d, s.e[0] = %d", s.e[2], s.e[1], s.e[0]);
      if ((s.e[2] != 1'b1) || (s.e[1] != 1'b1) || (s.e[0] != 1'b0)) begin
         $display("FAILED");
         $finish;
      end
      $display("PASSED");
   end

endmodule
