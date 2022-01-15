module main;

  reg [31:0] a, b, c;

  initial begin
    a = 1;
    b = 1;
    b[2] = 1'bx;
    c = a << b;
    //$display( "a: %b, b: %b, c: %b", a, b, c );
    if (c != 32'bx) $display("FAILED");
    else $display("PASSED");
  end
endmodule
