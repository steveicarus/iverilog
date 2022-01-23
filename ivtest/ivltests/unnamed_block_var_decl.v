// Check variable declarations in unnamed blocks
// All of these should pass in SystemVerilog and all but the last should fail in
// Verilog

module test;

initial begin
  integer x;
end

initial begin
  integer x;
  integer y;
end

initial begin
  integer x, y;
end

initial begin
  integer x;
  integer y;
  x = y;
end

initial begin
  $display("PASSED");
end

endmodule
