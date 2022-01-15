module a;
  initial begin : b
    reg x;
  end
  initial fork : c
    reg x;
  join

  initial begin
    a.b.x = 1'b0;
    a.c.x = 1'b1;
    $display("PASSED");
  end
endmodule
