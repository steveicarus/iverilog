`define ADDR_DEC_W	      8                 // Number of bits used to decode.
`define ADDR_DEVICE0	     `ADDR_DEC_W'h10    // Device 0 located at address 20xx_xxxxh
`define ADDR_DEVICE1	     `ADDR_DEC_W'h1F    // Device 1 located at address 20xx_xxxxh

module top ( ) ;

// Instantiation of the module
//
child_module #(`ADDR_DEC_W, `ADDR_DEVICE0, `ADDR_DEVICE1) my_module ( );

initial begin
  #1 ;
end

endmodule

module child_module ( );

// Parameters:
parameter		dec_addr_w = 4 ;
parameter		t0_addr    = 4'd0 ;
parameter		t1_addr    = 4'd0 ;

// Instantiation of the grandchild module
//
grandchild_module #(dec_addr_w, t0_addr, t1_addr) my_grandchild_module ( );

initial begin
  $display ("CHILD parameters are: %h %h %h", dec_addr_w, t0_addr, t1_addr) ;
end

endmodule

module grandchild_module ( );

// Parameters:
parameter		dec_addr_w = 4 ;
parameter		t0_addr    = 4'd0 ;
parameter		t1_addr    = 4'd0 ;

initial begin
  $display ("GRANDCHILD parameters are: %h %h %h", dec_addr_w, t0_addr, t1_addr) ;
end

endmodule
