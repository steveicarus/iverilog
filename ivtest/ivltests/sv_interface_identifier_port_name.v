// Check that an ordinary port can match a visible interface name.

interface I;
endinterface

module dut(
  input wire I,
  output wire value
);

  assign value = I;

endmodule

module test;

  wire value;

  dut i_dut(.I(1'b1), .value(value));

  initial begin
    #1;

    if (value !== 1'b1) begin
      $display("FAILED");
      $finish;
    end

    $display("PASSED");
  end

endmodule
