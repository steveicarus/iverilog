module test (d, en, g, s, a);

  input  [31:2]   d;
  input           en;
  output          g, s, a;

  reg             g, s, a;
  reg    [31:21]  r14;
  reg    [2:0]    r18;

  always @(d or r18 or r14 or en) begin
    casex ({d[31:12],r18[2:0],en})
     {20'b1111_1111_0011_????_????, 3'b???, 1'b1} : s = 1'b1;
     {20'b1111_1111_0010_????_????, 3'b???, 1'b1} : g = 1'b1;
     {{r14[31:21], 9'b0_01??_????}, 3'b???, 1'b?} : a = 1'b1;
     {{r14[31:21], 9'b1_????_????}, 3'b???, 1'b?} : a = 1'b1;
   endcase
  end

  // Other tests check functionality so if this compiles it is fine.
  initial $display("PASSED");

endmodule
