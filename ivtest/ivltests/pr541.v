module example;
    reg  [2:0] rhs;
    wire [5:0] lhs = - rhs;
    integer    ix;
    initial begin
        $monitor( "%b[5:0] = - %b[2:0]", lhs, rhs );
        for ( ix = 0; ix <= 7; ix = ix + 1 ) begin
            rhs = ix;
            #100 ;
        end
    end
endmodule
