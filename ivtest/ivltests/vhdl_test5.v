//
// Author: Pawel Szostek (pawel.szostek@cern.ch)
// Date: 01.08.2011

`timescale 1ns/1ps

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
    wire [7:0] i, o;
    wire [0:7] o_vl;
    dummy dummy_vhdl(o, i);
    stimulus stim(i);
    assign o_vl = i;

    always @(i) begin
    #1;
        if(o !== o_vl) begin
            $display("OUTPUT: ", o);
            $display("INPUT: ", i);
            $display("CORRECT: ", o_vl);
        end
    end
    initial begin
        #120000;
            $display("PASSED");
        $finish;
    end
endmodule
