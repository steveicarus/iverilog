`begin_keywords "1364-2005"
`timescale 1ns/100ps

module top;
  parameter pdly = 1.2;
  real rdly = 1.3;
  integer idly = 1;
  reg in = 1'b0;
  wire gi, gf, gs, gt;

  wire #idly int = in;
  wire #1.1 first = in;
  wire #pdly second = in;
  wire #rdly third = in;

  buf #idly (gi, in);
  buf #1.1 (gf, in);
  buf #pdly (gs, in);
  buf #rdly (gt, in);

  initial begin
    $monitor($realtime,, int,, first,, second,, third,, gi,, gf,, gs,, gt);
    #0 in = 1'b1;
    #2 in = 1'b0;
    #4;
    rdly = -6.1; // Since we are at 6 this will not wrap.
    in = 1'b1;
    @(third or gt) $display("Large delay: ", $realtime);
  end

  initial #1.1 $display("Should be 1.1: ", $realtime);  // This should be 1.1
  initial #pdly $display("Should be 1.2: ", $realtime);  // This should be 1.2
  initial begin
    #0; // We need this so that rdly has a defined value.
    #rdly $display("Should be 1.3: ", $realtime);  // This should be 1.3
  end
  initial begin
    #0; // We need this so that rdly has a defined value.
    #idly $display("Should be 1.0: ", $realtime);  // This should be 1.0
  end
endmodule

`timescale 1ns/1ps
module top2;
  initial #1.001 $display("Should be 1.001: ", $realtime);
endmodule
`end_keywords
