`timescale 1ns/100ps

module assign_test;


reg     clk;
reg     cat1;
reg     cat2;
reg     cat3;
reg     cat4;
reg     foo1;
reg     foo2;
reg     foo3;
reg     foo4;
reg     bar1;
reg     bar2;
reg     bar3;
reg     bar4;


initial begin
    clk = 0;
    #100 $finish(0);
end


always begin
    clk = 0;
    #50;
    clk = 1;
    #50;
end


always @(posedge clk) begin
    cat1  = #1 1;
    cat2  = #1 1;
    cat3  = #1 1;
    cat4  = #1 1;
    foo1  = #1 1;
    foo2  = #1 1;
    foo3  = #1 1;
    foo4  = #1 1;
    bar1 <= #1 1;
    bar2 <= #1 1;
    bar3 <= #1 1;
    bar4 <= #1 1;
end


always @(cat1)
  $write("time=%0t, cat1=%0h\n", $time, cat1);

always @(cat2)  $write("time=%04d, cat2=%0h\n", $time, cat2);

always @(cat3)  $write("time=%04d, cat3=%0h\n", $time, cat3);

always @(cat4)  $write("time=%04d, cat4=%0h\n", $time, cat4);

always @(foo1)  $write("time=%04d, foo1=%0h\n", $time, foo1);

always @(foo2)  $write("time=%04d, foo2=%0h\n", $time, foo2);

always @(foo3)  $write("time=%04d, foo3=%0h\n", $time, foo3);

always @(foo4)  $write("time=%04d, foo4=%0h\n", $time, foo4);

always @(bar1)  $write("time=%04d, bar1=%0h\n", $time, bar1);

always @(bar2)  $write("time=%04d, bar2=%0h\n", $time, bar2);

always @(bar3)  $write("time=%04d, bar3=%0h\n", $time, bar3);

always @(bar4)  $write("time=%04d, bar4=%0h\n", $time, bar4);


endmodule
