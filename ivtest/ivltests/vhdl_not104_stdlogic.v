module check (input unsigned [103:0] a, c);
  wire [103:0] int_AB;

  assign int_AB = ~a;

always @(a, int_AB, c) begin
  #1;
  if (int_AB !== c) begin
     $display("ERROR");
     $finish;
  end
end

endmodule

module stimulus (output reg unsigned [103:0] A);
  parameter S = 2000;
  int unsigned i;


  initial begin
    // values with 0, 1
    for (i=0; i<S; i=i+1) begin
       #1;
       A[103:8] = {$random, $random, $random};
       A[7:0] = $random % 256;
    end
    // values with x, z
    for (i=0; i<S; i=i+1) begin
       #1;
       A[103:8] = {$random, $random, $random};
       A[7:0] = $random % 256;
       A[103:72] = xz_inject (A[103:72]);
       A[71:40]  = xz_inject (A[71:40]);
    end
end

  // injects some x, z values on 32 bits arguments
  function [31:0] xz_inject (input unsigned [31:0] value);
      integer i, temp;
      begin
        temp = {$random};
        for (i=0; i<32; i=i+1)
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
  wire unsigned [103:0] a;
  wire unsigned [103:0] r;

  stimulus     stim           (.A(a));
  not104       duv            (.a_i(a), .c_o(r) );
  check        check          (.a(a), .c(r) );

  initial begin
    #20000;
    $display("PASSED");
    $finish;
  end

endmodule
