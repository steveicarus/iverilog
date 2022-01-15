module test();

wire [3:0] foo;
wire [5:0] bar;

assign foo[2:1] = 2'b10;
assign bar[4:1] = foo;

initial begin
  if (bar === 6'bzz10zz)
    $display("PASSED");
  else
    $display("PASSED");
end

endmodule
