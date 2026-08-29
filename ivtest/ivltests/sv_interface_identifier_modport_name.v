// Check that a modport can match the interface name.

interface I;
  logic value;
  modport I(output value);
endinterface

module dut(I.I bus);

  assign bus.value = 1'b1;

endmodule

module test;

  I bus();

  dut i_dut(bus);

  initial begin
    #1;

    if (bus.value !== 1'b1) begin
      $display("FAILED");
      $finish;
    end

    $display("PASSED");
  end

endmodule
