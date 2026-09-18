// Check that a function imported into the compilation unit takes precedence
// over a function in an enclosing instance.

package p;

  function integer value;
    value = 1;
  endfunction

endpackage

module child;

  initial begin
    if (value() == 1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule

import p::value;

module test;

  child i_child();

  function integer value;
    value = 2;
  endfunction

endmodule
