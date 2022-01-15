module m;

  reg [15:0] x,y;

  initial
  begin
    y = 0;
    x <= 0;
    x <= 1;
    x <= 2;
    x <= 3;
    x <= 4;
    x <= 5;
    x <= 6; // Only this should cause an event, so y should become 1.
    #10
    $display("x = %d (expect 6)", x);
    $display("y = %d (expect 1)", y);
    if (x !== 16'd6) begin
       $display("FAILED");
       $finish;
    end
    if (y !== 16'd1) begin
       $display("FAILED");
       $finish;
    end
     $display("PASSED");
    $finish;
  end

  always @(x[0])
    y <= y + 1;

endmodule
