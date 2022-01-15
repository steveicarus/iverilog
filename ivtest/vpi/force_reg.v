module test();

reg  [1:0] IN;
wire [1:0] OUT;

assign OUT = IN;

initial begin
  #1 $peek(IN);
  #0 $display("display : %b", OUT);
  #1 $force(IN);
  #1 $peek(IN);
  #0 $display("display : %b", OUT);
  #1 $release(IN);
  #0 $display("display : %b", OUT);
  #1 $force(IN);
  #1 $peek(IN);
  #0 $display("display : %b", OUT);
  #1 $poke(IN);
  #1 $peek(IN);
  #0 $display("display : %b", OUT);
  #1 $release(IN);
  #0 $display("display : %b", OUT);
  #1 $poke(IN);
  #1 $peek(IN);
  #0 $display("display : %b", OUT);
end

endmodule
