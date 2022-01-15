module example();

reg [7:0] scale, a, b;

wire [7:0] c;

function [7:0] scaled;

input [7:0] value;

begin
  scaled = value * scale;
end

endfunction

assign c = scaled(a) + scaled(b);

initial begin
  #1 a = 2;
  #1 scale = 2;
  #1 b = 3;
  #1 $display(c);
  if (c === 10)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
