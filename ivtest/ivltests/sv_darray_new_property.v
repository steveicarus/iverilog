// Check that a dynamic array constructor accepts a class property.

module test;

  int a[];

  class C;
    int data[];
  endclass

  C c;

  initial begin
    c = new;
    a = new[1];
    a[0] = 42;
    c.data = a;
    a = new[1](c.data);

    if (a[0] !== 42) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule
