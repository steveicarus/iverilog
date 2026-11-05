module mult8_8 (
    input  wire       clk,
    input  wire [7:0] sw_a,
    input  wire [7:0] sw_b,
    output wire [3:0] digit,
    output reg  [7:0] Seven_Segment
);
    // ---------------------------------------------------------
    // 1. Multiply (pure combinational, always live)
    // ---------------------------------------------------------
    wire [15:0] product = sw_a * sw_b;          // max 255*255 = 65025

    // ---------------------------------------------------------
    // 2. Binary to 4-digit decimal (clamp to 9999 so no wraparound
    //    confusion if product exceeds 4 digits)
    // ---------------------------------------------------------
    wire [15:0] disp_val = (product > 16'd9999) ? 16'd9999 : product;

    wire [3:0] d_thousands = disp_val / 1000;
    wire [3:0] d_hundreds  = (disp_val / 100) % 10;
    wire [3:0] d_tens      = (disp_val / 10)  % 10;
    wire [3:0] d_ones      =  disp_val % 10;

    // ---------------------------------------------------------
    // 3. Segment lookup (active-low, common anode: 0 = segment ON)
    //    bit order: {A,B,C,D,E,F,G,DP}
    // ---------------------------------------------------------
    function [7:0] seg7(input [3:0] v);
        case (v)
            4'd0: seg7 = 8'b00000011;
            4'd1: seg7 = 8'b10011111;
            4'd2: seg7 = 8'b00100101;
            4'd3: seg7 = 8'b00001101;
            4'd4: seg7 = 8'b10011001;
            4'd5: seg7 = 8'b01001001;
            4'd6: seg7 = 8'b01000001;
            4'd7: seg7 = 8'b00011111;
            4'd8: seg7 = 8'b00000001;
            4'd9: seg7 = 8'b00001001;
            default: seg7 = 8'b11111111; // blank
        endcase
    endfunction

    // ---------------------------------------------------------
    // 4. Digit refresh — clean synchronous 2-bit counter driving
    //    a ONE-HOT active-low digit enable, updated once per tick
    //    (avoids any combinational race/glitch on digit lines)
    // ---------------------------------------------------------
   reg [15:0] tick_div;
reg [1:0]  active_digit;

always @(posedge clk) begin
    tick_div <= tick_div + 1'b1;

    if (tick_div == 16'hFFFF)
        active_digit <= active_digit + 1'b1;
end

    // one-hot active-LOW enable — only ONE bit is 0 at any time
   assign digit = 4'b1000 >> active_digit;
    // ---------------------------------------------------------
    // 5. Segment output follows whichever digit is currently active
    // ---------------------------------------------------------
    always @(*) begin
        case (active_digit)
            2'd0: Seven_Segment = seg7(d_ones);
            2'd1: Seven_Segment = seg7(d_tens);
            2'd2: Seven_Segment = seg7(d_hundreds);
            2'd3: Seven_Segment = seg7(d_thousands);
        endcase
    end
endmodule