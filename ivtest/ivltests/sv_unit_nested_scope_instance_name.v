// Check that an enclosing instance name does not affect compilation-unit
// lookup from a nested function scope.

parameter integer value = 29;

module holder;
endmodule

module child;

  function integer read_value;
    read_value = value;
  endfunction

  initial begin
    if (read_value() !== 29) begin
      $display("FAILED: value=%0d", read_value());
    end else begin
      $display("PASSED");
    end
  end

endmodule

module test;

  holder value();
  child child_instance();

endmodule
