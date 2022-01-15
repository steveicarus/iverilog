module test();

reg  [3:0] IN;
wire [3:0] OUT;

assign OUT = IN;

initial begin
  #1 $peek(IN[2:1]);
  #0 $display("display :%b", OUT);
  #1 $force(IN[2:1]);
  #1 $peek(IN[2:1]);
  #0 $display("display :%b", OUT);
  #1 $release(IN[2:1]);
  #0 $display("display :%b", OUT);
  #1 $force(IN[2:1]);
  #1 $peek(IN[2:1]);
  #0 $display("display :%b", OUT);
  #1 $poke(IN[2:1]);
  #1 $peek(IN[2:1]);
  #0 $display("display :%b", OUT);
  #1 $release(IN[2:1]);
  #0 $display("display :%b", OUT);
  #1 $poke(IN[2:1]);
  #1 $peek(IN[2:1]);
  #0 $display("display :%b", OUT);
end

endmodule
