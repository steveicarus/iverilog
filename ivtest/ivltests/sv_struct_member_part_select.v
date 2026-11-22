// Indexed part-selects of a packed struct member are valid l-values.

module test;

  typedef struct packed {
    logic [15:0] w;
    logic [7:0] x;
  } p_t;

  p_t p;
  reg failed;

  initial begin
    failed = 1'b0;
    p = '0;

    p.w[0 +: 8] = 8'h40;
    p.w[15 -: 8] = 8'hbf;
    if (p.w !== 16'hbf40) begin
      $display("FAILED: expected bf40, got %h", p.w);
      failed = 1'b1;
    end

    p.w[4 +: 8] = 8'h5a;
    if (p.w !== 16'hb5a0) begin
      $display("FAILED: expected b5a0, got %h", p.w);
      failed = 1'b1;
    end

    p.x[3 -: 4] = 4'ha;
    if (p.x !== 8'h0a) begin
      $display("FAILED: expected 0a, got %h", p.x);
      failed = 1'b1;
    end

    p.w[0 +: 8] += 8'h21;
    if (p.w !== 16'hb5c1) begin
      $display("FAILED: expected b5c1, got %h", p.w);
      failed = 1'b1;
    end

    if (!failed) $display("PASSED");
  end

endmodule
