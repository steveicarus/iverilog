package p;

  localparam a = 1, b = 2;

  typedef logic [b:a] vector_t;

endpackage

module a();

  localparam a = 1, b = 4;

  import p::*;

  vector_t v;

  initial begin:blk
    v = ~0;
    $display("%b", v);
    if (v === 2'b11)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
