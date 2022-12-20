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

/* comment */ `unconnected_drive /* comment */ pull0 /* comment */ // comment
module pulllow(in);
  input in;
  initial #1 if (in !== 1'b0) begin
    $display("FAILED: pull0 of floating input port (%b)", in);
    top.pass = 1'b0;
  end
endmodule
/* comment */ `nounconnected_drive /* comment */ // comment

/* comment */`unconnected_drive/*
   comment */
/* comment */pull1/*
   comment */
module pullhigh(in);
  input in;
  initial #1 if (in !== 1'b1) begin
    $display("FAILED: pull1 of floating input port (%b)", in);
    top.pass = 1'b0;
  end
endmodule
/* comment */`nounconnected_drive/*
   comment */
