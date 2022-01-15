module top;
  wire [2:-1] vec;
  integer idx;

  initial begin
    idx = 'bx;
    force vec[idx] = 1'b1;
    release vec[idx];
  end
endmodule
