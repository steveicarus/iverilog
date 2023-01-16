// Check that assigning to a enum types variable from an enum typed class
// array element works.

module test;

  typedef enum reg [31:0] {
    A, B
  } E;

  E e;
  E ea[2];

  initial begin
    ea[0] = B;
    e = ea[0];
    if (e === B) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
