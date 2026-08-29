// Check that an interface instance can match its interface type name.

interface I;
  logic value;
endinterface

module test;

  I I();

  initial begin
    I.value = 1'b1;
    #1;

    if (I.value !== 1'b1) begin
      $display("FAILED");
      $finish;
    end

    $display("PASSED");
  end

endmodule
