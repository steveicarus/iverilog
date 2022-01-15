module check (input signed [22:0] a, b, c);
  wire signed [22:0] int_AB;

  assign int_AB = a / b;

always @(a, b, int_AB, c) begin
  #1;
  if (int_AB != c) begin
     $display("ERROR");
     $finish;
  end
end

endmodule

module stimulus (output reg signed [22:0] A, B);
  parameter MAX = 1 << 23;
  parameter S = 10000;
  int unsigned i;


  initial begin
    A = 0; B= 1;
    for (i=0; i<S; i=i+1) begin
       #1 A = $random % MAX;
          B = $random % 500;
          if (B == 0) B = B + 1;
    end
    #1 A = 23'h7fffff;
    #1 B = 23'h7fffff;
    #1 A = -1;
    #1 B = 1;
  end

endmodule


module test;
  wire signed [22:0] a, b;
  wire signed [22:0] r;

  stimulus     stim           (.A(a), .B(b));
  sdiv23       duv            (.a_i(a), .b_i(b), .c_o(r) );
  check        check          (.a(a), .b(b), .c(r) );

  initial begin
    #20000;
    $display("PASSED");
    $finish;
  end

endmodule
