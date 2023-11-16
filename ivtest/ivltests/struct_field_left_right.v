module main;

   typedef struct packed {
      logic [15:8] f;
   } structtype;

   structtype s;

   initial
   begin
      $display("$left(s.f) = %2d, $right(s.f) = %2d", $left(s.f), $right(s.f));
      if ($left(s.f) !== 15) begin
         $display("FAILED -- $left(s.f) = %2d", $left(s.f));
         $finish;
      end
      if ($right(s.f) !== 8) begin
         $display("FAILED -- $right(s.f) = %2d", $right(s.f));
         $finish;
      end
      $display("PASSED");
   end

endmodule
