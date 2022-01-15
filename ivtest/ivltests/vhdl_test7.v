//
// Author: Pawel Szostek (pawel.szostek@cern.ch)
// Date: 01.08.2011

`timescale 1ns/1ps

module dummy_v( input [7:0] in, output reg [7:0] out);
    assign out = {in[7], 7'b1111111}; //there is no equivalent to vhdl's `others'
endmodule

module stimulus (output reg [7:0] a);
    parameter S = 20000;
    int unsigned j,i;
    initial begin
        for(i=0; i<S; i=i+1) begin
            #10;
            a[7] <= inject();
            a[6] <= inject();
            a[5] <= inject();
            a[4] <= inject();
            a[3] <= inject();
            a[2] <= inject();
            a[1] <= inject();
            a[0] <= inject();
        end
    end
    function inject();
        reg ret;
        reg unsigned [3:0] temp;
        temp[3:0] = $random % 16;
        begin
            if(temp >= 10)
                ret = 1'b1;
            else if(temp >= 4)
                ret = 1'b0;
            else if(temp >= 2)
                ret = 1'bx;
            else
                ret = 1'b0;
            inject = ret;
        end
    endfunction
endmodule
module main;
    wire [7:0] i,o;
    wire [7:0] veri;
    dummy dummy_vhdl(i,o);
    dummy_v dummy_verilog(i, veri);
    stimulus stim(i);


    always @(i) begin
    #1;
        if(o != veri) begin
            $display("ERROR!");
            $display("VERILOG: ", veri);
            $display("VHDL: ", o);
            $stop;
        end
    end
    initial begin
        #12000;
        #10;
            $display("PASSED");
        //stop;
    end
endmodule
