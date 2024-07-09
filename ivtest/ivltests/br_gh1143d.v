module test;

function integer count(input integer value);

integer i;

begin
  i = 0;
  for ( ; ; ) begin
    if (i == value) break;
    i = i + 1;
  end
  count = i;
end

endfunction

localparam integer c = count(10);

integer v;

initial begin
  v = count(20);
  $display(c,,v);
  if (c === 10 && v === 20)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
