// Check that an indexed part-select of a packed struct member can be the
// target of a continuous assignment.

module test;
  typedef struct packed { logic [15:0] w; } p_t;

  p_t p;
  logic [7:0] s = 8'h40;

  assign p.w[0 +: 8] = s;
  assign p.w[15 -: 8] = ~s;

  initial begin
    #1;
    if (p.w !== 16'hbf40) begin
      $display("FAILED: p.w=%h", p.w);
    end else begin
      $display("PASSED");
    end
  end
endmodule
