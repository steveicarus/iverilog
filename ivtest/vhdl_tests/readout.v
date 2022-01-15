// This causes GHDL to fail with 'port "p" cannot be read'
// since the code generator does not yet identify p as an
// internal signal as well as a port.
module top;
   wire ign;

   a inst(ign);

endmodule // top

module a(p);
   output p;

   b inst(p);

   initial begin
      #1;
      if (p !== 1)
        $display("FAILED -- p !== 1");
      else
        $display("PASSED");
      $finish;
   end

endmodule // a

module b(q);
   output q;

   assign q = 1;

endmodule // b
