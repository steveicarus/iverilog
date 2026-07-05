// Check that package names can shadow visible type identifiers.

package p;
  typedef int T;
endpackage

import p::*;

package T;
  parameter int VALUE = 23;
endpackage

module test;

  reg failed;

  initial begin
    failed = 1'b0;

    if (T::VALUE !== 23) begin
      $display("FAILED(%0d). Package name did not hide typedef", `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
