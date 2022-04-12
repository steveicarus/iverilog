module recursive_func();

function automatic [15:0] factorial;

input [15:0] n;

begin
  factorial = (n > 1) ? factorial(n - 1) * n : n;
end

endfunction

reg [15:0] r1;
reg [15:0] r2;
reg [15:0] r3;

initial begin
  fork
    r1 = factorial(3);
    r2 = factorial(4);
    r3 = factorial(5);
  join
  $display("factorial 3 = %0d", r1);
  $display("factorial 4 = %0d", r2);
  $display("factorial 5 = %0d", r3);
end

wire [15:0] r4;
wire [15:0] r5;
wire [15:0] r6;

assign r4 = factorial(6);
assign r5 = factorial(7);
assign r6 = factorial(8);

initial begin
  #1;
  $display("factorial 6 = %0d", r4);
  $display("factorial 7 = %0d", r5);
  $display("factorial 8 = %0d", r6);
end

endmodule
