module top;
   function real copy(input real r);
     copy = r;
   endfunction

   real a, b, c, d, e, f;
   initial begin
      a = 0.4;
      b = 0.5;
      c = 0.6;
      d = 2.4;
      e = 2.5;
      f = 2.6;
      $display("a: %.1f  %0d  %0x  %0b", copy(a), copy(a), copy(a), copy(a));
      $display("b: %.1f  %0d  %0x  %0b", copy(b), copy(b), copy(b), copy(b));
      $display("c: %.1f  %0d  %0x  %0b", copy(c), copy(c), copy(c), copy(c));
      $display("d: %.1f  %0d  %0x  %0b", copy(d), copy(d), copy(d), copy(d));
      $display("e: %.1f  %0d  %0x  %0b", copy(e), copy(e), copy(e), copy(e));
      $display("f: %.1f  %0d  %0x  %0b", copy(f), copy(f), copy(f), copy(f));
      a = -0.4;
      b = -0.5;
      c = -0.6;
      d = -2.4;
      e = -2.5;
      f = -2.6;
      $display("a: %.1f  %0d  %0x  %0b", copy(a), copy(a), copy(a), copy(a));
      $display("b: %.1f  %0d  %0x  %0b", copy(b), copy(b), copy(b), copy(b));
      $display("c: %.1f  %0d  %0x  %0b", copy(c), copy(c), copy(c), copy(c));
      $display("d: %.1f  %0d  %0x  %0b", copy(d), copy(d), copy(d), copy(d));
      $display("e: %.1f  %0d  %0x  %0b", copy(e), copy(e), copy(e), copy(e));
      $display("f: %.1f  %0d  %0x  %0b", copy(f), copy(f), copy(f), copy(f));
   end
endmodule
