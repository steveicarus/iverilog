module \esc.mod ;
  \esc.sub \esc.inm (1'b1);
  sub inst();
endmodule

module \esc.sub (input wire \esc.port );
  reg \esc.val ;
  reg normal;
  initial begin
    $print_if_found();
    normal = 1'b0;
    \esc.val = 1'b1;
  end
endmodule

module sub;
  reg \esc.id ;
  reg normal;
  initial begin
    normal = 1'b1;
    \esc.id = 1'b0;
  end
endmodule
