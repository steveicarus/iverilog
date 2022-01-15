/*
Verilog-XL error message:

Error!    Recursive instantiation of module (a)             [Verilog-RINOM]
          "murec.v", 13: a a0(z, x);
1 error
*/

module test;
    reg x ;
    wire z ;

    a a0( z, x );
endmodule


module a( z, x );
    output z ;
    input  x ;

    b b0( z, x );
endmodule


module b( z, x );
    output z ;
    input  x ;

    a a0( z, x ); // Error should be caught here
endmodule
