// Icarus doesn't properly support variable expressions on the right hand
// side of a procedural CA - see bug 605.

module test();

reg  [1:0] addr;
reg  [3:0] memory[3:0];
reg  [3:0] data;

initial begin
  assign data = memory[addr];
  addr = 1;
  memory[addr] = 2;
  #0 $display("%d", data);
  if (data === 2)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
