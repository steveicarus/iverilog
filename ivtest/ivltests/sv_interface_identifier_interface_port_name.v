// Check that an interface port can match its interface type name.

interface I;
  logic value;
endinterface

module dut(I I);

  assign I.value = 1'b1;

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
