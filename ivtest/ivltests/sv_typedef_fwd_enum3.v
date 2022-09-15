// Check that a forwarded enum typedef can be referenced in a class

module test;

  typedef T;

  class C;
    T x;
  endclass

  typedef enum integer {
    X, Y
  } T;

  initial begin
    C c;
    c = new;

    if ($bits(c.x) == 32) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
