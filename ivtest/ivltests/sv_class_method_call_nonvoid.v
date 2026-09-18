// Check that a bare inherited non-void function can be called as a statement.

class B;
  int calls;

  function int value();
    calls = calls + 1;
    return calls;
  endfunction
endclass

class C extends B;
  function void run();
    value();
  endfunction
endclass

module test;
  C object;

  initial begin
    object = new;
    object.calls = 0;
    object.run();

    if (object.calls !== 1) begin
      $display("FAILED: expected one call, got %0d", object.calls);
    end else begin
      $display("PASSED");
    end
  end
endmodule
