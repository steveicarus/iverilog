module testbench();
  reg [3:0] a, b;

  initial begin
    a = 1;
    b = 2;
    #1;
    a = a + b;
    b = a + b;
    if (a !== 3)
      begin
        $display("FAILED -- a !== 3");
        $finish;
      end
    if (b !== 5)
      begin
        $display("FAILED -- b !== 5");
        $finish;
      end
    #2;
    $display("PASSED");
  end // initial begin

  initial begin
    #2;
    if (a !== 3)
      begin
        $display("FAILED -- a (signal) !== 3");
        $finish;
      end
    if (b !== 5)
      begin
        $display("FAILED -- b (signal) !== 5");
        $finish;
      end
  end

endmodule // testbench
