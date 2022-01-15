module stimulus (output reg A, B);


  initial begin
    {A, B} = 2'b00;
    #10 {A, B} = 2'b01;
    #10 {A, B} = 2'b10;
    #10 {A, B} = 2'b11;
  end

endmodule

module scoreboard (input Y, A, B);

function truth_table (input a, b);
  reg [1:0] gate_operand;
  reg       gate_output;
  begin
    gate_operand[1:0] = {a, b};
    case (gate_operand)
        2'b00: gate_output = 0;
        2'b01: gate_output = 1;
        2'b10: gate_output = 1;
        2'b11: gate_output = 1;
      endcase

    truth_table = gate_output;
    end
endfunction


reg Y_t;

always @(A or B) begin
  Y_t = truth_table (A, B);
  #1;
  //$display ("a = %b, b = %b, Y_s = %b, Y = %b", A, B, Y_s, Y);
  if (Y_t !== Y) begin
      $display("FAILED! - mismatch found for inputs %b and %b in OR operation", A, B);
      $finish;
  end
end

endmodule

module test;
  stimulus     stim    (A, B);
  or_gate      duv     (.a_i(A), .b_i(B), .c_o(Y) );
  scoreboard   mon     (Y, A, B);

  initial begin
    #100;
    $display("PASSED");
    $finish;
  end

endmodule
