// $ is only allowed in the index of a queue.

module top;
  int q[$] = '{1, 2, 3};
  int d[] = '{1, 2, 3};
  int a[3] = '{1, 2, 3};
  logic [7:0] v;
  int x;

  initial begin
    x = q[v[$-1]];
    x = q[d[$-1]];
    x = q[a[$-1]];
    x = $signed($);
    x = {$, 1'b1};
    $display("FAILED");
  end
endmodule
