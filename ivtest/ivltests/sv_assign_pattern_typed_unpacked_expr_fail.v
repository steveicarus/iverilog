// Check that a typed pattern of an unpacked type is rejected as an operand.

module test;

  typedef int arr_t[2];
  logic [31:0] v;

  initial begin
    v = {arr_t'{1, 2}, 1'b0};
  end

endmodule
