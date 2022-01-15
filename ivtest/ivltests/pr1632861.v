module test;
  reg[9:0] tst;

  initial begin
    #1 tst = 0;
    // This should set the register to 10'b00000xxxxx!
    #1 tst = 5'hxx;
    #1 tst = 10'h3ff;
    #1 tst = 10'b00000xxxxx;
    #1 tst = 0;
    #1 tst = 8'hxx;
    #1 tst = 0;
    #1 $finish(0);
  end

  always @(tst) begin
    $display("At %0t value is %b", $time, tst);
  end
endmodule
