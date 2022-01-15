// Use the default timescale.
module top;
  initial begin
    $printtimescale(top);
    $printtimescale(other);
    $printtimescale(other2);
  end
endmodule

`timescale 1ms/1ms

// Use the given timescale.
module other;
endmodule

`resetall

// Use the default timescale.
module other2;
endmodule
