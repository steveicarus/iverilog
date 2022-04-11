// Check that constant recursive functions are supported when the `return`
// statement is used.

module recursive_func();

function automatic [15:0] factorial(input [15:0] n);
  if (n > 1) begin
    return factorial(n - 1) * n;
  end
  return n;
endfunction

localparam F3 = factorial(3);
localparam F4 = factorial(4);
localparam F5 = factorial(5);

initial begin
  $display("factorial 3 = %0d", F3);
  $display("factorial 4 = %0d", F4);
  $display("factorial 5 = %0d", F5);
end

endmodule
