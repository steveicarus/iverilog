module stimulus (output reg A, B);

  initial begin
    // both inputs are x
    #0  {A, B} = 2'bxx;
     // both inputs are z
    #10 {A, B} = 2'bzz;
    // one input is a zero
    #10 {A, B} = 2'b0x;
    #10 {A, B} = 2'bx0;
    #10 {A, B} = 2'b0z;
    #10 {A, B} = 2'bz0;
    // one input is a one
    #10 {A, B} = 2'b1x;
    #10 {A, B} = 2'bx1;
    #10 {A, B} = 2'b1z;
    #10 {A, B} = 2'bz1;
    // one input x, other z
    #10 {A, B} = 2'bxz;
    #10 {A, B} = 2'bzx;
   // normal bit operands
    #10 {A, B} = 2'b00;
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
        // both inputs are x
        2'bxx: gate_output = 1'bx;
        // both inputs are z
        2'bzz: gate_output = 1'bx;
        // output should be x (one input is a one)
        2'b1x: gate_output = 1'bx;
        2'bx1: gate_output = 1'bx;
        2'b1z: gate_output = 1'bx;
        2'bz1: gate_output = 1'bx;
        // output is x (one input is a zero)
        2'b0x: gate_output = 1'bx;
        2'bx0: gate_output = 1'bx;
        2'b0z: gate_output = 1'bx;
        2'bz0: gate_output = 1'bx;
        // inputs x, z
        2'bxz: gate_output = 1'bx;
        2'bzx: gate_output = 1'bx;
        // normal operation on bit
        2'b00: gate_output = 1;
        2'b01: gate_output = 0;
        2'b10: gate_output = 0;
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
      $display("FAILED! - mismatch found for inputs %b and %b in XNOR operation", A, B);
      $finish;
  end
end

endmodule

module test;
  stimulus     stim    (A, B);
  xnor_gate    duv     (.a_i(A), .b_i(B), .c_o(Y) );
  scoreboard   mon     (Y, A, B);

  initial begin
    #200;
    $display("PASSED");
    $finish;
  end

endmodule
