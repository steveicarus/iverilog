`timescale 1s/1ms

module dut;

reg [7:0] a, b;

endmodule

`timescale 1ms/1ms

module test;

dut dut();

initial begin
    dut.a = 0; dut.b = 0;
    $monitor_time_slot(2.0, dut.a, dut.b);
    $monitor_time_slot(5.0, dut.a, dut.b);
    #1000;
    dut.a = 1; dut.b <= 1;
    #1000;
    dut.a = 2; dut.b <= 2;
    #1000;
    dut.a = 3; dut.b <= 3;
    #1000;
    dut.a = 4; dut.b <= 4;
    #1000;
    dut.a = 5; dut.b <= 5;
    #1000;
    dut.a = 6; dut.b <= 6;
    #1000;
    $finish(0);
end

endmodule
