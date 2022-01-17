// Test that cast to enum works in procedural assignments
module test();

typedef enum { a, b, c } enum_type;

enum_type enum_value;

initial begin
  enum_value = enum_type'(1);

  if (enum_value == b) begin
    $display("PASSED");
  end else begin
    $display("FAILED");
  end
end

endmodule
