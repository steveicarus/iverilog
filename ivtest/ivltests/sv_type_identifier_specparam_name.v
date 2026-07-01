// Check that a specparam name can shadow a visible type identifier.

typedef int SP_DELAY;

module test(input in, output out);
  specify
    specparam SP_DELAY = 1;
    (in => out) = SP_DELAY;
  endspecify

  initial begin
    $display("PASSED");
  end
endmodule
