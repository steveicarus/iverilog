module example;

    reg  [7:0]  vec;
    reg  [3:0]  ix;
    wire        vix = vec[ix];

    initial begin
        $display( "                time   ix vix      vec" );
        $display( "                ---- ---- --- --------" );
        $monitor( "%T %b   %b %b", $time, ix, vix, vec );
        vec = 8'b00000000;
             ix = 0;                         //    0
        #100 ix = 1;                         //  100
        #100 ix = 2;                         //  200
        #100 ix = 3;                         //  300
        #100 ix = 4;                         //  400
        #100 ix = 5;                         //  500
        #100 ix = 6;                         //  600
        #100 ix = 7;                         //  700
        #100 ix = 8;                         //  800
        #100 ix = 4'b001x;                   //  900
        #100 ix = 4'b01x0;                   // 1000
        #100 ix = 4'b0x01;                   // 1100
        #100 ix = 0;                         // 1200
        #100                vec[ix] <= 1'b1;  // 1300
        #100                vec[ix] <= 1'b0;  // 1400
        #100 ix = 3;                         // 1500
        #100                vec[ix] <= 1'b1;  // 1600
        #100                vec[ix] <= 1'b0;  // 1700
        #100 ix = 6;                         // 1800
        #100                vec[ix] <= 1'b1;  // 1900
        #100                vec[ix] <= 1'b0;  // 2000
        #100 ix = 8;                         // 2100
        #100                vec[ix] <= 1'b1;  // 2200
        #100                vec[ix] <= 1'b0;  // 2300
        #100 ix = 4'b010x;                   // 2400
        #100                vec[ix] <= 1'b1;  // 2500
        #100                vec[ix] <= 1'b0;  // 2600
        #100 ix = 4'b00x1;                   // 2700
        #100                vec[ix] <= 1'b1;  // 2800
        #100                vec[ix] <= 1'b0;  // 2900
        #100 ix = 4'b0x10;                   // 3000
        #100                vec[ix] <= 1'b1;  // 3100
        #100                vec[ix] <= 1'b0;  // 3200
        #100 ix = 4'bxxxx;                   // 3300
        #100                vec[ix] <= 1'b1;  // 3400
        #100                vec[ix] <= 1'b0;  // 3500
        #100 $display( "Finish at time %T", $time );
    end

endmodule
