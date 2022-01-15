module nested_func();

function automatic real sum;

input real a;
input real b;

begin
  sum = a + b;
end

endfunction

real r1;
real r2;
real r3;

initial begin
  r1 = sum(sum(2, 3), sum(4, 5));
  r2 = sum(3, sum(4, sum(5, 6)));
  r3 = sum(sum(sum(4, 5), 6), 7);
  $display("sum of 2 to 5 = %0d", r1);
  $display("sum of 3 to 6 = %0d", r2);
  $display("sum of 4 to 7 = %0d", r3);
end

endmodule
