`timescale 1s/1ms

module test;

reg [7:0] a, b;

initial begin
    a = 0; b = 0;
    $monitor_time_slot(2000, a, b);
    $monitor_time_slot(5000, a, b);
    #1;
    a = 1; b <= 1;
    #1;
    a = 2; b <= 2;
    #1;
    a = 3; b <= 3;
    #1;
    a = 4; b <= 4;
    #1;
    a = 5; b <= 5;
    #1;
    a = 6; b <= 6;
    #1;
    $finish(0);
end

endmodule
