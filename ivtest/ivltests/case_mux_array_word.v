// Check that case statement muxes work with array word inputs.

module test;

  reg [7:0] mem [0:3];
  reg [1:0] sel;
  reg [7:0] out;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  always @* begin
    case (sel)
      2'd0: out = mem[0];
      2'd1: out = mem[1];
      2'd2: out = mem[2];
      2'd3: out = mem[3];
    endcase
  end

  (* ivl_synthesis_off *)
  initial begin
    failed = 1'b0;

    mem[0] = 8'h12;
    mem[1] = 8'h34;
    mem[2] = 8'h56;
    mem[3] = 8'h78;

    sel = 2'd0;
    #1 `check(out, 8'h12);

    sel = 2'd1;
    #1 `check(out, 8'h34);

    sel = 2'd2;
    #1 `check(out, 8'h56);

    sel = 2'd3;
    #1 `check(out, 8'h78);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
