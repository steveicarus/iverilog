// Test that cast to enum works in continuous assignments
module test();

typedef enum { a, b, c } enum_type;

enum_type enum_value;

assign enum_value = enum_type'(1);

initial begin
  if (enum_value == b) begin
    $display("PASSED");
  end else begin
    $display("FAILED");
  end
end

endmodule
