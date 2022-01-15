module m;

  reg [31:0] count = 0;

  function void func1();
    count++;
    if (count < 10) func2();
  endfunction

  function void func2();
    count++;
    if (count < 10) func1();
  endfunction

  initial begin
    func1();
    if (count == 10)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
