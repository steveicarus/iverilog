module pr2837451();

// this code provides a regression test that exercises
// vvp_fun_part_sa::recv_vec4_pv

reg  [3:0] a;
wire [7:0] b;
wire [3:0] c;
wire [3:0] d;
wire [3:0] e;

assign b[5:2] = a;

assign c = b[4:1];
assign d = b[5:2];
assign e = b[6:3];

initial begin
  a = 4'b0101;
  #1;
  $display("%b %b %b %b %b", a, b, c, d, e);
  if ((b === 8'bzz0101zz)
   && (c === 4'b101z)
   && (d === 4'b0101)
   && (e === 4'bz010))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
