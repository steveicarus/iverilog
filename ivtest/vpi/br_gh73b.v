module partsel(inout wire [1:0] part);

assign part = 2'bz;

endmodule


module test();

wire [3:0] full;

partsel sel(full[2:1]);

initial begin
  #1 $peek(full[2:1]);
  #0 $display("display : %b %b", sel.part, full);
  #1 $force(full[2:1]);
  #1 $peek(full[2:1]);
  #0 $display("display : %b %b", sel.part, full);
  #1 $release(full[2:1]);
  #0 $display("display : %b %b", sel.part, full);
  #1 $force(full[2:1]);
  #1 $peek(full[2:1]);
  #0 $display("display : %b %b", sel.part, full);
  #1 $poke(full[2:1]);
  #1 $peek(full[2:1]);
  #0 $display("display : %b %b", sel.part, full);
  #1 $release(full[2:1]);
  #0 $display("display : %b %b", sel.part, full);
  #1 $poke(full[2:1]);
  #1 $peek(full[2:1]);
  #0 $display("display : %b %b", sel.part, full);
end

endmodule
