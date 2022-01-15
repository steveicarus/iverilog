module pr2973532;

wire [15:0] a;
wire [7:0]  b;
wire [7:0]  c;

assign b[5:2] = 4'b1111;

assign c = 8'b00000000;

assign a = {b, c};

initial begin
  #1;
  $display("%b", a);
  if (a === 16'bzz1111zz00000000)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
