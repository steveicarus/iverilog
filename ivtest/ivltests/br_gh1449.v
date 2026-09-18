module top;
  logic passed;
  typedef logic [9:0] exp_t [0:7];

  function automatic exp_t build_exp();
    exp_t t;
    logic [9:0] a = 10'd1;
    for (int i = 0; i < 8; ++i) begin
      t[i] = a;
      a = a + 10'd1;
    end
    return t;
  endfunction

  localparam exp_t EXP = build_exp();

  initial begin
    passed = 1'b1;
    for (int i = 0; i < 8; ++i) begin
      if (i+1 != EXP[i]) begin
        $display("FAILED: expected %0d, got %0d", i+1, EXP[i]);
	passed = 1'b0;
      end
    end
    if (passed) $display("PASSED");
  end
endmodule
