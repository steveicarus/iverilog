module bug;

function [1:0] Copy;

input [1:0] Value;

begin
  Copy = Value;
end

endfunction

integer   i;
integer   j;

reg [1:0] Expect;
reg [1:0] Actual;
reg       Failed;

initial begin
  Failed = 0;
  for (i = 0; i < 4; i = i + 1) begin
    for (j = 0; j < 4; j = j + 1) begin
      Expect = (i%2)*2 + (j%2);
      Actual = Copy((i%2)*2 + (j%2));
      if (Actual !== Expect) begin
        $display("Failed: %0d,%0d expected %0d, got %0d", i, j, Expect, Actual);
        Failed = 1;
      end
    end
  end
  if (!Failed) $display("PASSED");
end

endmodule
