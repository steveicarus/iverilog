module top;
  parameter parm = 1.4;
  reg [31:0] str;

  initial begin
    $sformat(str, "R: %d", parm);
    if (str !== "   1") $display("FAILED: expected '   1', got %s", str);
    else $display("PASSED");
  end
endmodule
