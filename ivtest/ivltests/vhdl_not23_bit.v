module check (input unsigned [22:0] a, c);
  wire [22:0] int_AB;

  assign int_AB = ~a;

always @(a, int_AB, c) begin
  #1;
  if (int_AB != c) begin
     $display("ERROR");
     $finish;
  end
end

endmodule

module stimulus (output reg unsigned [22:0] A);
  parameter MAX = 1 << 23;
  parameter S = 10000;
  int unsigned i;


  initial begin
    A = 0;
    for (i=0; i<S; i=i+1) begin
       #1 A = {$random} % MAX;
    end
  end

endmodule


module test;
  wire unsigned [22:0] a;
  wire unsigned [22:0] r;

  stimulus     stim           (.A(a));
  not23        duv            (.a_i(a), .c_o(r) );
  check        check          (.a(a), .c(r) );

  initial begin
    #20000;
    $display("PASSED");
    $finish;
  end

endmodule
