// Check that power results can be assigned to two-state class properties.

class C;
  bit [7:0] value;
endclass

module test;
  bit [7:0] base = 0;
  bit signed [7:0] exponent = -1;
  C c;

  initial begin
    c = new;
    c.value = base ** exponent;
    if (c.value === 8'h00) begin
      $display("PASSED");
    end else begin
      $display("FAILED: expected 00, got %h", c.value);
    end
  end
endmodule
