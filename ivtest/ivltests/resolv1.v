module test;

wire w;
wire q;
reg g;

pullup(w);
bufif1(w, 1'b1, g);
pmos(q, w, 1'b0);
bufif0(q, 1'b0, g);

initial
  begin
    g = 1;
    #10
    $display(q, w);	// should print "11"
    #20
    g = 0;		// w changes from St1 to Pu1
    #30
    $display(q, w);	// should print "01"
    if (q == 1'b0)
      $display("PASSED");
    else
      $display("FAILED");
    #40
    $finish;
  end

endmodule
