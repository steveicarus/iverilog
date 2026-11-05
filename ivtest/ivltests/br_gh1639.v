// Check that an out of range constant select of a packed struct member
// reads as 'bx and that writing it has no effect.

module test;
  typedef struct packed {
    logic [3:0] a;
    logic [3:0] b;
    logic [3:0] c;
  } ps_t;

  typedef struct packed {
    logic [3:0] x;
    logic [1:0][3:0] p;
  } pa_t;

  ps_t s;
  pa_t t;
  reg failed = 1'b0;

  initial begin
    s = 12'hABC;
    t = 12'h123;

    if (s.b[0] !== 1'b1 || s.b[2] !== 1'b0 || s.b[2:1] !== 2'b01) begin
      $display("FAILED: in range select of s.b");
      failed = 1'b1;
    end
    if (s.b[4] !== 1'bx || s.b[5] !== 1'bx || s.b[-1] !== 1'bx) begin
      $display("FAILED: s.b[4]=%b s.b[5]=%b s.b[-1]=%b", s.b[4], s.b[5], s.b[-1]);
      failed = 1'b1;
    end
    if (s.a[4] !== 1'bx || s.c[-1] !== 1'bx) begin
      $display("FAILED: s.a[4]=%b s.c[-1]=%b", s.a[4], s.c[-1]);
      failed = 1'b1;
    end
    if (t.p[1] !== 4'h2 || t.p[2] !== 4'bxxxx) begin
      $display("FAILED: t.p[1]=%h t.p[2]=%b", t.p[1], t.p[2]);
      failed = 1'b1;
    end

    s.b[4] = 1'b1;
    s.b[-1] = 1'b0;
    t.p[2] = 4'hf;
    if (s !== 12'hABC || t !== 12'h123) begin
      $display("FAILED: out of range write changed s=%h t=%h", s, t);
      failed = 1'b1;
    end

    if (!failed) $display("PASSED");
  end
endmodule
