// Multiple declarations in one scope and both legal omitted-argument forms.
`timescale 1ps/1ps
module same_scope(input wire CLK);
  reg n10 = 0, n20 = 0;
  specify
    $width(posedge CLK, 10, 0, n10);
    $width(posedge CLK, 20, 0, n20);
  endspecify
endmodule
module omit_threshold(input wire CLK);
  specify $width(posedge CLK, 10); endspecify
endmodule
module omit_notifier(input wire CLK);
  specify $width(posedge CLK, 10, 4); endspecify
endmodule
module tb;
  reg CLK = 0;
  same_scope same(CLK);
  omit_threshold omit(CLK);
  omit_notifier threshold(CLK);
  integer n10_seen = 0, n20_seen = 0;
  reg armed = 0;
  initial begin #1; $width_audit; armed = 1; end
  always @(same.n10) if (armed) begin
    if ((n10_seen == 0 && ($time != 109 || same.n10 !== 1'b1)) ||
        (n10_seen == 1 && ($time != 303 || same.n10 !== 1'b0)) ||
        n10_seen >= 2) $fatal(1, "FAIL n10 history");
    n10_seen = n10_seen + 1;
  end
  always @(same.n20) if (armed) begin
    if ((n20_seen == 0 && ($time != 109 || same.n20 !== 1'b1)) ||
        (n20_seen == 1 && ($time != 210 || same.n20 !== 1'b0)) ||
        (n20_seen == 2 && ($time != 303 || same.n20 !== 1'b1)) ||
        n20_seen >= 3) $fatal(1, "FAIL n20 history");
    n20_seen = n20_seen + 1;
  end
  initial begin
    #100 CLK = 1;
    #9 CLK = 0;
    #91 CLK = 1;
    #10 CLK = 0;
    #90 CLK = 1;
    #3 CLK = 0;
    #47;
    if (n10_seen != 2 || n20_seen != 3) $fatal(1, "FAIL notifier count");
    $width_audit_finish;
    $display("PASSED");
    $finish;
  end
endmodule
