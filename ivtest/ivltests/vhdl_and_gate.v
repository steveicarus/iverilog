module stimulus (output reg A, B);
  reg unsigned [1:0] stimulus_count = 2'b0;

  initial begin
    {A, B} = 2'b00;
    #10 {A, B} = 2'b01;
    #10 {A, B} = 2'b10;
    #10 {A, B} = 2'b11;
  end

endmodule

module scoreboard (input Y, A, B);
  reg [3:0] truth_table;
  reg Y_s;

initial begin
  truth_table['b00] = 0;
  truth_table['b01] = 0;
  truth_table['b10] = 0;
  truth_table['b11] = 1;
end

always @(A or B) begin
  Y_s = truth_table[{A,B}];
  #1;
  //$display ("a = %b, b = %b, Y_s = %b, Y = %b", A, B, Y_s, Y);
  if (Y_s !== Y) begin
      $display("FAILED! - mismatch found for inputs %b and %b", A, B);
      $finish;
  end
end

endmodule

module test;
  stimulus     stim    (A, B);
  and_gate     duv     (.a_i(A), .b_i(B), .c_o(Y) );
  scoreboard   mon     (Y, A, B);

  initial begin
    #100 $display("PASSED");
    $finish;
  end

endmodule
