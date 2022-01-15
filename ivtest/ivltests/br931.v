module bug();

reg flag1;
reg flag2;

task task1(inout integer flag);

begin
  #4 flag = 1;
end

endtask

task task2(inout integer flag);

begin
  #5 flag = 1;
end

endtask

task task3(inout flag1, inout flag2);

fork
  task1(flag1);
  task2(flag2);
join

endtask

initial begin
  flag1 = 0;
  flag2 = 0;
  task3(flag1, flag2);
  $display("flag1 = %d", flag1);
  $display("flag2 = %d", flag2);
  if ((flag1 === 1) && (flag2 === 1))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
