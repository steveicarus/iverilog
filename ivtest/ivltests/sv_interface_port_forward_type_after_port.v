// Check that an attribute does not prevent recognizing a forward interface
// port type after another ANSI port declaration.

module dut(
  input wire enable,
  (* keep = 1 *) bus_if.producer bus,
  bus_if mirror
);

  assign bus.value = enable;
  assign mirror.value = bus.value;

endmodule

interface bus_if;
  logic value;

  modport producer(output value);
endinterface

module test;

  reg enable;
  bus_if bus();
  bus_if mirror();

  dut i_dut(enable, bus, mirror);

  initial begin
    enable = 1'b1;
    #1;

    if (bus.value !== 1'b1 || mirror.value !== 1'b1) begin
      $display("FAILED");
      $finish;
    end

    $display("PASSED");
  end

endmodule
