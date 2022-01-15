module test();

reg [3:0] count;
reg       carry;

reg failed = 0;

integer i;

initial begin
  {carry, count} = 0;
  for (i = 0; i < 32; i += 1) begin
    $display("%b %b", carry, count);
    if (count !== i[3:0]) failed = 1;
    if (carry !== i[4]) failed = 1;
    {carry, count} += 1;
  end

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
