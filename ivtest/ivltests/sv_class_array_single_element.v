// Check that a one-element class array can be assigned through an index.

module test;

  class C;
  endclass

  C a[1];
  C c;

  initial begin
    c = new;
    a[0] = c;
    c = a[0];

    if (c == null) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule
