`timescale 1 ps/1 ps

// extracted from altera_mf.v
module bug2011429;
    reg pass = 1'b1;
    reg [7:0] vco_tap;
    reg vco_c0_last_value;
    integer c_ph_val[0:5];

    always @(vco_tap[c_ph_val[0]])
        vco_c0_last_value = vco_tap[c_ph_val[0]];

    initial begin
        vco_tap = 8'b10101010;
        c_ph_val[0] = 0;
        #1;
        if (vco_c0_last_value != 1'b0) begin
            $display("FAILED initial value, got %b", vco_c0_last_value);
            pass = 1'b0;
        end

        vco_tap = vco_tap >> 1;
        #1;
        if (vco_c0_last_value != 1'b1) begin
            $display("FAILED shifted value, got %b", vco_c0_last_value);
            pass = 1'b0;
        end

        c_ph_val[0] = 1;
        #1;
        if (vco_c0_last_value != 1'b0) begin
            $display("FAILED index change, got %b", vco_c0_last_value);
            pass = 1'b0;
        end

        if (pass) $display("PASSED");
    end
endmodule
