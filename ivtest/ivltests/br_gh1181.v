module top_module();
  wire data_bus; // Bidirectional data bus

  module_a u_module_a (
    .data(data_bus)
  );

  module_b u_module_b (
    .data(data_bus)
  );
endmodule

module module_a (
  inout wire data
);
  // Drive the data bus with 1 when module_a is active
  assign data = 1'b1;
endmodule

module module_b (
  inout wire data
);
  // Read data from the bus
  wire data_in = data;

  // Set data to high impedance to allow other modules to drive the bus
  assign data = 1'bz;
endmodule

module tb();

top_module dut();

initial begin
  #0;
  if (dut.data_bus === 1'b1)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
