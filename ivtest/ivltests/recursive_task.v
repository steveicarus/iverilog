module recursive_task();

task automatic factorial;

input  integer n;
output integer f;

integer t;

fork
  begin
    if (n > 1)
      factorial(n - 1, t);
    else
      t = 1;
    #1 f = n * t;
  end
  begin
    @f $display("intermediate value = %0d", f);
  end
join

endtask

integer r1;
integer r2;
integer r3;

initial begin
  fork
    factorial(3, r1);
    factorial(4, r2);
    factorial(5, r3);
  join
  $display("factorial 3 = %0d", r1);
  $display("factorial 4 = %0d", r2);
  $display("factorial 5 = %0d", r3);
end

endmodule
