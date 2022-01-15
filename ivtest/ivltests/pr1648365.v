module ivitest;

reg clock_1x, clock_2x;

reg [31:1] w [0:1];

wire [31:1] w0 = w[0];
wire [31:1] w1 = w[1];

initial begin
    #0; //avoid time-0 race
    clock_1x = 0;
    clock_2x = 0;
    $monitor($time,,"w0=%h, w1=%h", w0, w1);
    #1;
    forever begin
        clock_1x = !clock_1x;
        clock_2x = !clock_2x;
        #5;
        clock_2x = !clock_2x;
        #5;
    end
end

reg phase;
always @(clock_1x) begin
    phase = #1 clock_1x;
end

reg [31:1] u;

always @(posedge clock_2x) begin
    u <= 'haaaaaaaa;
end


reg [31:1] v;

always @(posedge clock_2x) begin
    v <= 'h99999999;
    if (phase) begin
        w[0] <= v;
        w[1] <= u;
    end
end


reg [31:1] x0, x1;

always @(posedge clock_1x) begin
    x0 <= w[0];
    x1 <= w[1];
end


initial begin
//    $dumpfile( "test.vcd" );
//    $dumpvars;

    #100;
    $finish(0);
end


endmodule
