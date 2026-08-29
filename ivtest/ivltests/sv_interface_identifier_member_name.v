// Check that an interface member can match the interface name.

interface I;
  logic I;
endinterface

module test;

  I bus();

  initial begin
    bus.I = 1'b1;
    #1;

    if (bus.I !== 1'b1) begin
      $display("FAILED");
      $finish;
    end

    $display("PASSED");
  end

endmodule
