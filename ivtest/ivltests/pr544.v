module example;
    wire    y;
    reg     p01, p01g, s01, s01g;
    bufif1  (pull0,   pull1  ) ( y, p01, p01g );
    bufif1  (strong0, strong1) ( y, s01, s01g );

    initial begin
        $monitor( "%T Pu:%b/%b St:%b/%b Y:%b,%v",
                  $time, p01, p01g, s01, s01g, y, y );
             { p01, p01g, s01, s01g } = 4'b0000;
        #100 { p01, p01g, s01, s01g } = 4'b0x00;
        #100 { p01, p01g, s01, s01g } = 4'b000x;
        #100 { p01, p01g, s01, s01g } = 4'b1x00;
        #100 { p01, p01g, s01, s01g } = 4'b001x;
        #100 { p01, p01g, s01, s01g } = 4'b0100;
        #100 { p01, p01g, s01, s01g } = 4'b0001;
        #100 { p01, p01g, s01, s01g } = 4'b1100;
        #100 { p01, p01g, s01, s01g } = 4'b0011;
        #100 { p01, p01g, s01, s01g } = 4'bx100;
        #100 { p01, p01g, s01, s01g } = 4'b00x1;
        #100 { p01, p01g, s01, s01g } = 4'b010x;
        #100 { p01, p01g, s01, s01g } = 4'bx10x;
        #100 { p01, p01g, s01, s01g } = 4'b111x;
        #100 { p01, p01g, s01, s01g } = 4'bx11x;
        #100 ;
    end
endmodule
