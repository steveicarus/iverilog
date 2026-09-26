// Check an enumeration method case expression with a wider case item.

module test;

  typedef enum bit [1:0] { A, B } enum_t;
  enum_t value;

  initial begin
    value = A;

    case (value.next())
      64'd1: $display("PASSED");
      default: $display("FAILED");
    endcase
  end

endmodule
