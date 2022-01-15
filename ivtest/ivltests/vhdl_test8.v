//
// Author: Pawel Szostek (pawel.szostek@cern.ch)
// Date: 01.08.2011

`timescale 1ns/1ps
module match_bits_v(input [7:0] a,b, output reg [7:0] match);
    integer i;
    wire ab_xor;
    always @(a or b) begin
        for (i=7; i>=0; i=i-1) begin
            match[i] = ~(a[i]^b[i]);
        end
    end
endmodule

module check(input [7:0] a,b,o_vhdl, o_verilog);

always @(a or b) begin
    #1 if (o_vhdl !== o_verilog) begin
       $display("ERROR!");
       $display("VERILOG: ", o_verilog);
       $display("VHDL: ", o_vhdl);
       $finish;
    end
end
endmodule

module stimulus (output reg [7:0] a,b);
    parameter S = 20000;
    int unsigned i,j,k,l;
    initial begin //stimulate data
        for (i=0; i<S; i=i+1) begin
            #5;
            for(k=0; k<8; k=k+1) begin
                a[k] <= inject();
            end
        end
    end

    initial begin //stimulate data
        for (i=0; i<S; i=i+1) begin
            #4;
            for(l=0; l<8; l=l+1) begin
                b[l] <= inject();
            end
        end
       #100 $display("PASSED");
       $finish;
    end

    function inject();
        reg [3:0] temp;
        begin
            temp = $random % 16;
            if(temp >= 10)
                inject = 1'b1;
            else
                inject = 1'b0;
        end
    endfunction
endmodule

module main;
    wire [7:0] a,b,o_vhdl, o_verilog;

    match_bits match_vhdl(a,b,o_vhdl);
    match_bits_v match_verilog(a,b,o_verilog);
    stimulus stim(a,b);
    check c(a,b,o_vhdl, o_verilog);
endmodule
