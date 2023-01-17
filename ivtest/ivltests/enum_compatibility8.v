// Check that assigning to a enum types variable from an enum typed class
// property works.

module test;

  typedef enum reg [31:0] {
    A, B
  } E;

  class C;
    E e;
  endclass

  C c;
  E e;

  initial begin
    c = new;
    c.e = B;
    e = c.e;
    if (e === B) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
