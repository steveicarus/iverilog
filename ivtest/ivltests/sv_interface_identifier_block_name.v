// Check that a procedural block can match a visible interface name.

interface I;
endinterface

module test;

  initial begin : I
    $display("PASSED");
  end

endmodule
