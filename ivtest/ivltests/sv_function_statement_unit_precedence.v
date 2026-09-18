// Check that a compilation-unit function takes precedence over a function in
// an enclosing instance when called as a statement.

integer result;

module child;

  initial begin
    result = 0;
    value();

    if (result == 1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule

function void value;
  result = 1;
endfunction

module test;

  child i_child();

  function void value;
    result = 2;
  endfunction

endmodule
