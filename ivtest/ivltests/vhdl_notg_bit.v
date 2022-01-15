module stimulus (output reg A);

  initial begin
    A = 1'b0;
    #10 A = 1'b1;
  end

endmodule

module scoreboard (input Y, A);

function truth_table (input a);
  reg gate_operand;
  reg       gate_output;
  begin
    gate_operand = a;
    case (gate_operand)
        1'b0: gate_output = 1;
        1'b1: gate_output = 0;
    endcase

    truth_table = gate_output;
    end
endfunction


reg Y_t;

always @(A) begin
  Y_t = truth_table (A);
  #1;
  //$display ("a = %b, b = %b, Y_s = %b, Y = %b", A, B, Y_s, Y);
  if (Y_t !== Y) begin
      $display("FAILED! - mismatch found for inputs %b in NOT operation", A);
      $finish;
  end
end

endmodule

module test;
  stimulus     stim    (A);
  not_gate     duv     (.a_i(A), .c_o(Y) );
  scoreboard   mon     (Y, A);

  initial begin
    #100;
    $display("PASSED");
    $finish;
  end

endmodule
