module test;

reg [6*8-1:0] \!"escaped"!\ ;

initial begin
  \!"escaped"!\ = "FAILED";

  $poke_escaped;

  $display("%s", \!"escaped"!\ );
end

endmodule
