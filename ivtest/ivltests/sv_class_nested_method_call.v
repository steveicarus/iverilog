// Check that an unqualified class method call works from a named block in
// another method.

module test;

  class C;
    function int get_value();
      return 1;
    endfunction

    function int call_method();
      begin : nested
        return get_value();
      end
    endfunction
  endclass

  C object;

  initial begin
    object = new;

    if (object.call_method() != 1) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule
