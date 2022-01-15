module top;
  wire real plus, minus;

  assign plus = 3.0;
  assign minus = -3.0; // This does not generate a Cr<>, so it core dumps.

  initial begin
    #1 $display(plus,, minus);
  end
endmodule
