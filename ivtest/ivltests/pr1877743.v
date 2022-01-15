`timescale 1ns/10ps
module top;
  reg pass = 1'b1;
  reg a, b, ci;
  wire co, s;
  adder dut(co, s, a, b, ci);

  initial begin
    // The initial value propagates in 1.6 nS.
    #1.59 check_result(1'bx, 1'bx, 1);

    #1 a = 0; b = 0; ci = 0; // 1.7
    #1.69 check_result(1'bx, 1'b0, 2);

    // Check the a => s delays.
    #1 a = 1; b = 0; ci = 0; // 1.1
    #1.09 check_result(1'b0, 1'b1, 3);
    #1 a = 0; b = 0; ci = 0; // 1.9
    #1.89 check_result(1'b1, 1'b0, 4);

    // Check the b => s delays.
    #1 a = 0; b = 1; ci = 0; // 1.2
    #1.19 check_result(1'b0, 1'b1, 5);
    #1 a = 0; b = 0; ci = 0; // 1.8
    #1.79 check_result(1'b1, 1'b0, 6);

    // Check the ci => s delays.
    #1 a = 0; b = 0; ci = 1; // 1.3
    #1.29 check_result(1'b0, 1'b1, 7);
    #1 a = 0; b = 0; ci = 0; // 1.7
    #1.69 check_result(1'b1, 1'b0, 8);

    // Check the a => s delays (state-dependent).
    #1 a = 0; b = 1; ci = 0;
    #3 a = 1; b = 1; ci = 0; // 2.0
    #1.99 check_result(1'b1, 1'b0, 9);
    #1 a = 0; b = 1; ci = 0; // 1.0
    #0.99 check_result(1'b0, 1'b1, 10);

    #1 a = 0; b = 1; ci = 1;
    #3 a = 1; b = 1; ci = 1; // 1.4
    #1.39 check_result(1'b0, 1'b1, 11);
    #1 a = 0; b = 1; ci = 1; // 1.6
    #1.59 check_result(1'b1, 1'b0, 12);

    // Check the co delay.
    #1 a = 0; b = 1; ci = 0;
    #1.49;
    if (co !== 1'b1) begin
      $display("Failed initial value for co test, %b != 1'b1", co);
      pass = 1'b0;
    end
    #0.02;
    if (co !== 1'b0) begin
      $display("Failed final value for co test, %b != 1'b0", co);
      pass = 1'b0;
    end

    #10 if (pass) $display("PASSED");
  end

  task check_result(input cur, input next, input integer num);
    begin
      if (s !== cur) begin
        $display("Failed initial value for test %0d, %b != %b", num, s, cur);
        pass = 1'b0;
      end
      #0.02;
      if (s !== next) begin
        $display("Failed final value for test %0d, %b != %b", num, s, next);
        pass = 1'b0;
      end
    end
  endtask
endmodule

module adder (co, s, a, b, ci);
  input a, b, ci;
  output co, s;

  assign {co, s} = a + b + ci;

  specify
    (a, b, ci => co) = 1.5;
    (ci => s) = (1.3, 1.7);
    if (b === 1'b1 && ci === 1'b0) (a => s) = (1, 2);
    if (b === 1'b1 && ci === 1'b1) (a => s) = (1.4, 1.6);
    ifnone (a => s) = (1.1, 1.9);
    ifnone (b => s) = (1.2, 1.8);
  endspecify
endmodule
