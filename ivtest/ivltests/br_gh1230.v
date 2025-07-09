module Foo #(parameter logic F = 1'b1) (input logic i = F);
  initial begin
    #1;
    $display("%m has an input value of %b.", i);
    if (i !== F) begin
      $display("FAILED: %m.i = %b ,expected %b!", i, F);
      Test.passed = 1'b0;
    end
  end
endmodule

module Test;
  reg passed;
  initial passed = 1'b1;

// FIXME: A default value is not currently supported for
//        a module instance array.
//  defparam e[0].F = 1'b0;
//  Foo e[1:0]();
  defparam g.F = 1'b0;
  Foo f(), g();
  Foo #(.F(1'bz)) h();

  final if (passed) $display("PASSED");
endmodule
