module test();

function parity(input bit_array[3:0]);

integer i;

begin
  parity = 0;
  for (i = 0; i < 4; i = i + 1) begin
    $display("%b", bit_array[i]);
    parity ^= bit_array[i];
  end
end

endfunction

reg a[3:0];
reg p;

initial begin
  a[0] = 1;
  a[1] = 0;
  a[2] = 1;
  a[3] = 1;
  p = parity(a);
  if (p === 1)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
