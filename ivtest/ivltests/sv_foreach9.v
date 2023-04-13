// Check that foreach counts from $left to $right for static arrays.

module test;

  logic [0:1][2:0] x[0:6][4:0][11];

  initial begin
    foreach(x[i,j,k,l,n]) begin
      $display(i, j, k, l, n);
    end
  end

endmodule
