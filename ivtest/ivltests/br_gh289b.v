package p;

  localparam a = 1, b = 2;

  typedef enum { A = a, B = b } enum_t;

endpackage

module a();

  localparam a = 3, b = 4;

  import p::*;

  enum_t e;

  initial begin
    e = A;
    $display(e);
    if (e === 1)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
