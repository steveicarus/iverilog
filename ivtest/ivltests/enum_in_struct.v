// Check that when a enum type is declared inside a struct that the enum is
// properly installed in the scope and the enum items are available

module test;

struct packed {
  enum integer {
    A
  } e;
} s;

initial begin
  s.e = A;
  if (s.e == A) begin
    $display("PASSED");
  end else begin
    $display("FAILED");
  end
end

endmodule
