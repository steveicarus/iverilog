module top;

  parameter cond = 1;
  parameter value = 25;
  parameter test = (cond) ? 5: 0;

  defparam dut.lwrval = (cond == 1) ? 6: (100/value) + 0.5;

  lower dut();

endmodule

module lower;
  parameter lwrval = 4;
  initial if (lwrval != 6.0) $display("FAILED"); else $display("PASSED");
endmodule
