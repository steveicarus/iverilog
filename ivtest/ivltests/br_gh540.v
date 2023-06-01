module top(
  pi_wi,    // port implicit, wire implicit
  pi_ws,    // port implicit, wire signed
  pi_wu,    // port implicit, wire unsigned

  ps_wi,    // port signed,   wire implicit
  ps_ws,    // port signed,   wire signed
  ps_wu,    // port signed,   wire unsigned

  pu_wi,    // port unsigned, wire implicit
  pu_ws,    // port unsigned, wire signed
  pu_wu     // port unsigned, wire unsigned
);

output pi_wi;
output pi_ws;
output pi_wu;

output signed ps_wi;
output signed ps_ws;
output signed ps_wu;

output unsigned pu_wi;
output unsigned pu_ws;
output unsigned pu_wu;

wire pi_wi = 1'b1;
wire ps_wi = 1'b1;
wire pu_wi = 1'b1;

wire signed pi_ws = 1'b1;
wire signed ps_ws = 1'b1;
wire signed pu_ws = 1'b1;

wire unsigned pi_wu = 1'b1;
wire unsigned ps_wu = 1'b1;
wire unsigned pu_wu = 1'b1;

reg [1:0] value;

reg failed = 0;

initial begin
  #1;

  value = pi_wi; $display("%b", value); if (value !== 2'b01) failed = 1;
  value = pi_ws; $display("%b", value); if (value !== 2'b11) failed = 1;
  value = pi_wu; $display("%b", value); if (value !== 2'b01) failed = 1;

  value = ps_wi; $display("%b", value); if (value !== 2'b11) failed = 1;
  value = ps_ws; $display("%b", value); if (value !== 2'b11) failed = 1;
  value = ps_wu; $display("%b", value); if (value !== 2'b11) failed = 1;

  value = pu_wi; $display("%b", value); if (value !== 2'b01) failed = 1;
  value = pu_ws; $display("%b", value); if (value !== 2'b11) failed = 1;
  value = pu_wu; $display("%b", value); if (value !== 2'b01) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
