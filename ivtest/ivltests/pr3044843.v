module top;
  parameter ab = 8;
  parameter ch = 2;

  reg [63:0] r;

  initial begin
    r[0+:ab * ch] = 2;
    if (r !== 64'hxxxxxxxxxxxx0002) $display("Failed, got %h", r);
    else $display("PASSED");
  end
endmodule
