module top;
  reg [2:-1] vec;
  integer idx;

  initial begin
    idx = 'bx;
    assign vec[idx+:1] = 1'b1;
    deassign vec[idx+:1];
  end
endmodule
