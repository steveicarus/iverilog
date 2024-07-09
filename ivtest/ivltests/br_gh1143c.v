module test;

function integer count(input integer value);

integer i;

begin
  for (i = 0 ; i < value; ) begin
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
