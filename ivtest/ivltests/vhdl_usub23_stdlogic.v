module check (input unsigned [22:0] a, b, c);
  wire unsigned [22:0] int_AB;

  assign int_AB = a - b;

always @(a, b, int_AB, c) begin
  #1;
  if (int_AB !== c) begin
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
    // x and z injected on A
    for (i=0; i<S/2; i=i+1) begin
       #1 A = {$random} % MAX;
          A = xz_inject (A);
    end
    // x and z injected on B
    #1 A = 1;
    for (i=0; i<S/2; i=i+1) begin
       #1 B = {$random} % MAX;
          B = xz_inject (B);
    end
    // x and z injected on A, B
    for (i=0; i<S; i=i+1) begin
       #1 A = {$random} % MAX;
          B = {$random} % MAX;
          A = xz_inject (A);
          B = xz_inject (B);
    end

  end

  // injects some x, z values on 23 bits arguments
  function [22:0] xz_inject (input unsigned [22:0] value);
      integer i, temp;
      begin
        temp = {$random};
        for (i=0; i<23; i=i+1)
          begin
             if (temp[i] == 1'b1)
               begin
                 temp = $random;
                 if (temp <= 0)
                    value[i] = 1'bx;  // 'x noise
                 else
                    value[i] = 1'bz;  // 'z noise
               end
          end
          xz_inject = value;
      end
  endfunction


endmodule


module test;
  wire unsigned [22:0] a, b;
  wire unsigned [22:0] r;

  stimulus     stim           (.A(a), .B(b));
  usub23       duv            (.a_i(a), .b_i(b), .c_o(r) );
  check        check          (.a(a), .b(b), .c(r) );

  initial begin
    #40000;
    $display("PASSED");
    $finish;
  end

endmodule
