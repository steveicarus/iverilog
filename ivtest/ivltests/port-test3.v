/***********************************************************************

  Incorrect direction non-detection test case
  Copyright (C) 2001  Eric LaForest, ecl@pet.dhs.org
  Licenced under GPL

***********************************************************************/

module CPU (data, address, rw, clock, reset);
        inout [15:0] data;
        output [15:0] address;
        // This should be an output really....
        input rw;
        input clock, reset;

        reg [15:0] data, address; // XXX error on data
        reg rw; // error on rw

        // I presume these should not be allowed to occur....
        always @(posedge clock) begin
                rw <= 1'b1;
        end

        always @(negedge clock) begin
                rw <= 1'b0;
        end

endmodule

module BENCH ();

        reg [15:0] address, data;
        reg rw, clock, reset;

        CPU fm (address, data, rw, clock, reset);

        initial begin
                clock <= 0;
                reset <= 1;
                #1000;
                $finish;
        end

        always begin
                # 10 clock <= ~clock;
        end

endmodule
