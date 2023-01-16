// Check that assigning to a enum types variable from an enum typed struct
// member works.

module test;

  typedef enum reg [31:0] {
    A, B
  } E;

  struct packed {
    E e;
  } s;

  E e;

  initial begin
    s.e = B;
    e = s.e;
    if (e === B) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
