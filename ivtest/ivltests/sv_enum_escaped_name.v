// Check that an enum literal name can contain a double quote.

module test;
  enum logic [4:0] {\value"name = 23} value;

  initial begin
    value = \value"name ;

    if (value !== 23) $display("FAILED");
    else $display("PASSED");
  end
endmodule
