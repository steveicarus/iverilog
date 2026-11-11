// Check that nested blocks can shadow the enclosing function's name.

module test;

  function integer f(input integer value);
    f = value;
    begin : inner
      integer f;
      f = 3;
    end
    begin
      integer f;
      f = 4;
    end
    begin : outer
      begin : f
      end
    end
  endfunction

  initial begin
    if (f(7) === 7) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
