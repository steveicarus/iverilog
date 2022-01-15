module m;

  reg [15:8] r;
  integer i;

initial
  begin
    r = 8'b01101001;
    for (i = 8; i <= 15; i = i + 1)
      $display(r[i]);
  end

endmodule
