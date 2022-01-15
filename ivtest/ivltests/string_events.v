module test();

string str;

always @(str) begin
  $display("str = %s", str);
end

task automatic test_task(input integer delay, input string str1, input string str2);

string str;

fork
  begin
    @(str) $display("str%0d = %s", delay, str);
    @(str) $display("str%0d = %s", delay, str);
  end
  begin
    #delay;
    #2 str = str1;
    #2 str = str1;
    #2 str = str2;
  end
join

endtask

initial begin
  #1 str = "hello";
  #1 str = "hello";
  #1 str = "world";
  fork
    test_task(1, "hello1", "world1");
    test_task(2, "hello2", "world2");
  join
  fork
    test_task(1, "world1", "hello1");
    test_task(2, "world2", "hello2");
  join
  #1 $finish(0);
end

endmodule
