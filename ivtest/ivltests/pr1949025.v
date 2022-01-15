module main;
  reg [2:0] value;

  initial begin
    for(value = 0; value <= 6; value = value + 1) begin
      $displayh(value,, 1<<(6-value));
    end
  end
endmodule
