module copy(inout [1:0] out, inout [1:0] in);

assign out = in;

endmodule

module top();

reg  [2:0] r;
wire [1:0] i1;
wire [1:0] i2;
wire [1:0] i3;
wire [0:0] o1;
wire [1:0] o2;
wire [2:0] o3;

assign i1 = r;
assign i2 = r;
assign i3 = r;

copy copy1(o1, i1);
copy copy2(o2, i2);
copy copy3(o3, i3);

reg failed;

initial begin
  failed = 0;
  for (r = 0; r < 4; r = r + 1) begin
    #1 $display("%b : %b %b : %b %b : %b %b", r[1:0], i1, o1, i2, o2, i3, o3);
    if (o1 !== r[0])           failed = 1;
    if (o2 !== r[1:0])         failed = 1;
    if (o3 !== {1'bz, r[1:0]}) failed = 1;
  end
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
