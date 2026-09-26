// Check null as a default function argument.

class C;
endclass

module test;

  function bit is_null(C value = null);
    return value == null;
  endfunction

  initial begin
    if (is_null())
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
