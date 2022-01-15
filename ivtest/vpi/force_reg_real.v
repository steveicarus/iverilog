module test();

real      IN;
wire real OUT;

assign OUT = IN;

initial begin
  #1 $peek(IN);
  #0 $display("display : %f", OUT);
  #1 $force(IN);
  #1 $peek(IN);
  #0 $display("display : %f", OUT);
  #1 $release(IN);
  #0 $display("display : %f", OUT);
  #1 $force(IN);
  #1 $peek(IN);
  #0 $display("display : %f", OUT);
  #1 $poke(IN);
  #1 $peek(IN);
  #0 $display("display : %f", OUT);
  #1 $release(IN);
  #0 $display("display : %f", OUT);
  #1 $poke(IN);
  #1 $peek(IN);
  #0 $display("display : %f", OUT);
end

endmodule
