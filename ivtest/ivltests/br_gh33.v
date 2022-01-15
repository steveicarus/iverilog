// Regression test for GitHub issue #33.

module tb;

    reg [3:0] mem [0:15] [0:15];

    task cycle;
        input [3:0] a, b, c;
        reg [3:0] tmp;
        begin
            tmp = mem[a][b];
            mem[a][b] = c;
            $display("a=%d, b=%d, c=%d -> old=%d, new=%d", a, b, c, tmp, mem[a][b]);
        end
    endtask

    initial begin
        cycle( 7, 0, 1);
        cycle(15, 0, 2);
        cycle( 7, 0, 3);
        cycle(15, 0, 4);
    end

endmodule
