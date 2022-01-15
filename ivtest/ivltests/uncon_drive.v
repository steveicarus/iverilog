module top;
  reg pass;

  highz dutz();
  pulllow dut0();
  pullhigh dut1();

  initial begin
    pass = 1'b1;
    #10;
    if (pass) $display("PASSED");
  end
endmodule

module highz(in);
  input in;
  initial #1 if (in !== 1'bz) begin
    $display("FAILED: high-Z of floating input port (%b)", in);
    top.pass = 1'b0;
  end
endmodule

`unconnected_drive pull0
module pulllow(in);
  input in;
  initial #1 if (in !== 1'b0) begin
    $display("FAILED: pull0 of floating input port (%b)", in);
    top.pass = 1'b0;
  end
endmodule
`nounconnected_drive

`unconnected_drive pull1
module pullhigh(in);
  input in;
  initial #1 if (in !== 1'b1) begin
    $display("FAILED: pull1 of floating input port (%b)", in);
    top.pass = 1'b0;
  end
endmodule
`nounconnected_drive
