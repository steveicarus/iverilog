module mult8_8_tb;
    reg        clk = 0;
    reg  [7:0] a, b;
    wire [3:0] digit;
    wire [7:0] seg;
    integer errors = 0;

    mult8_8 #(.DIV_BITS(2)) dut (.clk(clk), .sw_a(a), .sw_b(b),
                                 .digit(digit), .Seven_Segment(seg));
    always #5 clk = ~clk;

    function [7:0] enc(input integer v);
        case (v)
            0: enc = 8'b00000011; 1: enc = 8'b10011111;
            2: enc = 8'b00100101; 3: enc = 8'b00001101;
            4: enc = 8'b10011001; 5: enc = 8'b01001001;
            6: enc = 8'b01000001; 7: enc = 8'b00011111;
            8: enc = 8'b00000001; 9: enc = 8'b00001001;
            default: enc = 8'hFF;
        endcase
    endfunction

    // Wait until the given digit select is active, then check segments.
    task check(input [7:0] x, input [7:0] y);
        integer p, i, expv;
        reg [3:0] sel;
        begin
            a = x; b = y;
            p = x * y;
            if (p > 9999) p = 9999;
            for (i = 0; i < 4; i = i + 1) begin
                sel = 4'b0001 << i;
                // digit i (0=ones) is selected when digit == 4'b1000 >> idx
                // idx = i  ->  digit = 4'b1000 >> i
                sel = 4'b1000 >> i;
                while (digit !== sel) @(posedge clk);
                #1;
                case (i)
                    0: expv = p % 10;
                    1: expv = (p / 10) % 10;
                    2: expv = (p / 100) % 10;
                    3: expv = p / 1000;
                endcase
                if (seg !== enc(expv)) begin
                    $display("FAIL: %0d*%0d digit%0d seg=%b exp=%b", x, y, i, seg, enc(expv));
                    errors = errors + 1;
                end
            end
        end
    endtask

    initial begin
        check(0,0);   check(1,1);   check(12,34);  check(99,99);
        check(100,99); check(100,100); check(255,255); check(7,8);
        if (errors == 0) $display("PASSED");
        else             $display("FAILED: %0d errors", errors);
        $finish;
    end
endmodule
