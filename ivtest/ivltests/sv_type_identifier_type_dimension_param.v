// Check packed dimensions on typedef names in type parameter values.

package p;
  typedef logic [7:0] byte_t;
endpackage

typedef logic [3:0] nibble_t;

module M #(
  parameter integer WIDTH = 2,
  parameter type T = nibble_t [WIDTH-1:0]
);

  T value;

endmodule

module test;

  M i_default();
  M #(.WIDTH(3)) i_width();
  M #(2, p::byte_t [3:0]) i_ordered();
  M #(.T(p::byte_t [2:0])) i_override();
  reg failed;

  initial begin
    failed = 1'b0;

    if ($bits(i_default.value) !== 8) begin
      $display("FAILED(%0d). Expected 8, got %0d", `__LINE__,
               $bits(i_default.value));
      failed = 1'b1;
    end
    if ($bits(i_width.value) !== 12) begin
      $display("FAILED(%0d). Expected 12, got %0d", `__LINE__,
               $bits(i_width.value));
      failed = 1'b1;
    end
    if ($bits(i_ordered.value) !== 32) begin
      $display("FAILED(%0d). Expected 32, got %0d", `__LINE__,
               $bits(i_ordered.value));
      failed = 1'b1;
    end
    if ($bits(i_override.value) !== 24) begin
      $display("FAILED(%0d). Expected 24, got %0d", `__LINE__,
               $bits(i_override.value));
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
