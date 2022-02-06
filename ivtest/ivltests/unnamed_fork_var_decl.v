// Check variable declarations in unnamed forks
// All of these should pass in SystemVerilog and all should fail in Verilog

module test;

initial fork
  integer x;
join

initial fork
  integer x;
  integer y;
join

initial fork
  integer x, y;
join

initial fork
  integer x;
  integer y;
  x = y;
join

initial begin
  $display("PASSED");
end

endmodule
