// Check that an enclosing instance name does not affect compilation-unit
// variable lookup for a procedural l-value.

integer value = 0;

module holder;
endmodule

module child;

  initial begin
    value = 23;

    if (value !== 23) begin
      $display("FAILED: value=%0d", value);
    end else begin
      $display("PASSED");
    end
  end

endmodule

module test;

  holder value();
  child child_instance();

endmodule
