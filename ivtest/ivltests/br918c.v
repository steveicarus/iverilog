module driver(
  inout wire b0,
  inout wire b1
);

reg [1:0] v;

buf (strong0, pull1) buf0(b0, v[0]);
buf (strong0, pull1) buf1(b1, v[1]);

initial begin
  v = 2'b10;
end

endmodule


module br918c();

wire [1:0] bus;

pullup pu0(bus[0]);
pullup pu1(bus[1]);

driver driver(
  .b0 (bus[0]),
  .b1 (bus[1])
);

initial begin
  #1 $display("%b", bus);
  if (bus === 2'b10)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
