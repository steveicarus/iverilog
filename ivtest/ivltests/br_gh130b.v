// This is currently unsupported, but is legal code.
module test();

typedef enum { a, b, c } enum_type;

enum_type enum_value;

initial begin
  enum_value = enum_type'(1);
end

endmodule
