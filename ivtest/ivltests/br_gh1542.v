// Check that a 2-state enum wider than 64 bits is supported.

module test;
  typedef enum bit [64:0] {
    LOW = 65'h1,
    HIGH = 65'h1_0000_0000_0000_0001
  } code_t;

  code_t value;
  bit failed = 1'b0;

  initial begin
    value = HIGH;
    if (value !== 65'h1_0000_0000_0000_0001 || value.name() != "HIGH") begin
      $display("FAILED: value=%h name=%s", value, value.name());
      failed = 1'b1;
    end
    if (value.next() !== LOW || value.prev() !== LOW) begin
      $display("FAILED: next=%h prev=%h", value.next(), value.prev());
      failed = 1'b1;
    end
    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
