module top;
   real a, b, c, d, e, f;
   initial begin
      a = 0.4;
      b = 0.5;
      c = 0.6;
      d = 2.4;
      e = 2.5;
      f = 2.6;
      $display("a: %.1f  %0d  %0x  %0b", a, a, a, a);
      $display("b: %.1f  %0d  %0x  %0b", b, b, b, b);
      $display("c: %.1f  %0d  %0x  %0b", c, c, c, c);
      $display("d: %.1f  %0d  %0x  %0b", d, d, d, d);
      $display("e: %.1f  %0d  %0x  %0b", e, e, e, e);
      $display("f: %.1f  %0d  %0x  %0b", f, f, f, f);
      a = -0.4;
      b = -0.5;
      c = -0.6;
      d = -2.4;
      e = -2.5;
      f = -2.6;
      $display("a: %.1f  %0d  %0x  %0b", a, a, a, a);
      $display("b: %.1f  %0d  %0x  %0b", b, b, b, b);
      $display("c: %.1f  %0d  %0x  %0b", c, c, c, c);
      $display("d: %.1f  %0d  %0x  %0b", d, d, d, d);
      $display("e: %.1f  %0d  %0x  %0b", e, e, e, e);
      $display("f: %.1f  %0d  %0x  %0b", f, f, f, f);
   end
endmodule
