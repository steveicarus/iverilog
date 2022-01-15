module example;

    reg  [3:0]  mem [0:7];
    reg  [3:0]  addr;
    wire [3:0]  m0    = mem[0];
    wire [3:0]  m1    = mem[1];
    wire [3:0]  m2    = mem[2];
    wire [3:0]  m3    = mem[3];
    wire [3:0]  m4    = mem[4];
    wire [3:0]  m5    = mem[5];
    wire [3:0]  m6    = mem[6];
    wire [3:0]  m7    = mem[7];
    wire [3:0]  maddr = mem[addr];

    initial begin
        $write( "                " );
        $display(
"time addr maddr   m0   m1   m2   m3   m4   m5   m6   m7" );
        $write( "                " );
        $display(
"---- ---- ----- ---- ---- ---- ---- ---- ---- ---- ----" );
        $monitor( "%T %b  %b %b %b %b %b %b %b %b %b",
                  $time, addr, maddr,
                  m0, m1, m2, m3, m4, m5, m6, m7 );
        mem[0] = 8;
        mem[1] = 1;
        mem[2] = 2;
        mem[3] = 3;
        mem[4] = 4;
        mem[5] = 5;
        mem[6] = 6;
        mem[7] = 7;
             addr = 0;                       //    0
        #100 addr = 1;                       //  100
        #100 addr = 2;                       //  200
        #100 addr = 3;                       //  300
        #100 addr = 4;                       //  400
        #100 addr = 5;                       //  500
        #100 addr = 6;                       //  600
        #100 addr = 7;                       //  700
        #100 addr = 8;                       //  800
        #100 addr = 4'b001x;                 //  900
        #100 addr = 4'b01x0;                 // 1000
        #100 addr = 4'b0x01;                 // 1100
        #100 addr = 0;                       // 1200
        #100                mem[addr] =  9;  // 1300
        #100 addr = 3;                       // 1400
        #100                mem[addr] = 10;  // 1500
        #100 addr = 6;                       // 1600
        #100                mem[addr] = 11;  // 1700
        #100 addr = 8;                       // 1800
        #100                mem[addr] = 12;  // 1900
        #100 addr = 4'b010x;                 // 2000
        #100                mem[addr] = 13;  // 2100
        #100 addr = 4'b00x1;                 // 2200
        #100                mem[addr] = 14;  // 2300
        #100 addr = 4'b0x10;                 // 2400
        #100                mem[addr] = 15;  // 2500
        #100 addr = 4'bxxxx;                 // 2600
        #100                mem[addr] =  0;  // 2700
        #100 $display( "Finish at time %T", $time );
    end

endmodule
