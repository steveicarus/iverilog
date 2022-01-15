module partsel(inout wire [1:0] part);

assign part = 2'bz;

endmodule


module test();

wire [3:0] full;

partsel sel(full[1:0]);

initial begin
  #1 $peek(full);
  #0 $display("display : %b %b", full, sel.part);
  #1 $force(full);
  #1 $peek(full);
  #0 $display("display : %b %b", full, sel.part);
  #1 $release(full);
  #0 $display("display : %b %b", full, sel.part);
  #1 $force(full);
  #1 $peek(full);
  #0 $display("display : %b %b", full, sel.part);
  #1 $poke(full);
  #1 $peek(full);
  #0 $display("display : %b %b", full, sel.part);
  #1 $release(full);
  #0 $display("display : %b %b", full, sel.part);
  #1 $poke(full);
  #1 $peek(full);
  #0 $display("display : %b %b", full, sel.part);
end

endmodule
