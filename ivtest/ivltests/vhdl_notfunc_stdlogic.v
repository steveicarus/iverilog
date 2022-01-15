module stimulus (output reg A);

  initial begin
    // input is x
    #0  A = 1'bx;
     // input is z
    #10 A = 1'bz;
    // one input is a zero
    #10 A = 1'b0;
    // one input is a one
    #10 A = 1'b1;
  end

endmodule

module scoreboard (input Y, A);

function truth_table (input a);
  reg       gate_operand;
  reg       gate_output;
  begin
    gate_operand = a;
    case (gate_operand)
        // input is x
        1'bx: gate_output = 1'bx;
        // inputs is z
        1'bz: gate_output = 1'bx;
        // normal operation on bit
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
  if (Y_t !== Y) begin
      $display("FAILED! - mismatch found for input %b in NOT operation", A);
      $finish;
  end
end

endmodule

module test;
  stimulus     stim    (A);
  not_func     duv     (.a_i(A), .c_o(Y) );
  scoreboard   mon     (Y, A);

  initial begin
    #200;
    $display("PASSED");
    $finish;
  end

endmodule
