module check (input unsigned [22:0] a, b, input unsigned [45:0] c);
  wire unsigned [45:0] int_AB;

  assign int_AB = a * b;

always @(a, b, int_AB, c) begin
  #1;
  if (int_AB != c) begin
     $display("ERROR");
     $finish;
  end
end

endmodule

module stimulus (output reg unsigned [22:0] A, B);
  parameter MAX = 1 << 23;
  parameter S = 10000;
  int unsigned i;


  initial begin
    A = 0; B= 0;
    for (i=0; i<S; i=i+1) begin
       #1 A = {$random} % MAX;
          B = {$random} % MAX;
    end
    #1 A = 0;
       B = 0;
    #1 A = 23'h7fffff;
    #1 B = 23'h7fffff;
    #1 B = 0;
    #1 A = 1;
    #1 B = 1;
  end

endmodule


module test;
  wire unsigned [22:0] a, b;
  wire unsigned [45:0] r;

  stimulus     stim           (.A(a), .B(b));
  umul23       duv            (.a_i(a), .b_i(b), .c_o(r) );
  check        check          (.a(a), .b(b), .c(r) );

  initial begin
    #20000;
    $display("PASSED");
    $finish;
  end

endmodule
