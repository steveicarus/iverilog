module top;
  reg pass;
  time change;
  reg in;
  wire c1, c2a, c2b, c3;
  wire v1, v2, v3;

  const_1 d_c1(c1, in);
  const_2a d_c2a(c2a, in);
  const_2b d_c2b(c2b, in);
  const_3 d_c3(c3, in);

  var_1 d_v1(v1, in);
//  var_2 d_v2(v2, in);
  var_3 d_v3(v2, in);

  initial begin
    pass = 1'b1;
    #1000 in = 1'b0;
    #1000 in = 1'b1;
    #1000 in = 1'b0;
    #1000 in = 1'b1;
    #1000 in = 1'bx;
    #1000 in = 1'b0;
    #1000 in = 1'bx;
    #1000 in = 1'b1;
    #1000 in = 1'b0;
    #1000 if (pass) $display("PASSED");
  end

  always @(in) change = $time;
endmodule

// All delays should be 200.
module const_1 (output out, input in);
  assign #(200) out = (in === 1'bx) ? 1'bz : ~in;

  always @(out) begin
    case (out)
      1'b0: if ($time - top.change != 200) begin
        $display("Failed const_1 fall");
        top.pass = 1'b0;
      end
      1'b1: if ($time - top.change != 200) begin
        $display("Failed const_1 rise");
        top.pass = 1'b0;
      end
      1'bz: if ($time - top.change != 200) begin
        $display("Failed const_1 high-Z");
        top.pass = 1'b0;
      end
      default: begin
        $display("FAILED const_1 default");
        top.pass = 1'b0;
      end
    endcase
  end
endmodule

// Decay should also be 100.
module const_2a (output out, input in);
  assign #(200, 100) out = (in === 1'bx) ? 1'bz : ~in;

  always @(out) begin
    case (out)
      1'b0: if ($time - top.change != 100) begin
        $display("Failed const_2a fall");
        top.pass = 1'b0;
      end
      1'b1: if ($time - top.change != 200) begin
        $display("Failed const_2a rise");
        top.pass = 1'b0;
      end
      1'bz: if ($time - top.change != 100) begin
        $display("Failed const_2a high-Z");
        top.pass = 1'b0;
      end
      default: begin
        $display("FAILED const_2a default");
        top.pass = 1'b0;
      end
    endcase
  end
endmodule

// Decay should also be 100.
module const_2b (output out, input in);
  assign #(100, 200) out = (in === 1'bx) ? 1'bz : ~in;

  always @(out) begin
    case (out)
      1'b0: if ($time - top.change != 200) begin
        $display("Failed const_2b fall");
        top.pass = 1'b0;
      end
      1'b1: if ($time - top.change != 100) begin
        $display("Failed const_2b rise");
        top.pass = 1'b0;
      end
      1'bz: if ($time - top.change != 100) begin
        $display("Failed const_2b high-Z");
        top.pass = 1'b0;
      end
      default: begin
        $display("FAILED const_2b default");
        top.pass = 1'b0;
      end
    endcase
  end
endmodule

// All delays as given.
module const_3 (output out, input in);
  assign #(100, 200, 300) out = (in === 1'bx) ? 1'bz : ~in;

  always @(out) begin
    case (out)
      1'b0: if ($time - top.change != 200) begin
        $display("Failed const_3 fall");
        top.pass = 1'b0;
      end
      1'b1: if ($time - top.change != 100) begin
        $display("Failed const_3 rise");
        top.pass = 1'b0;
      end
      1'bz: if ($time - top.change != 300) begin
        $display("Failed const_3 high-Z");
        top.pass = 1'b0;
      end
      default: begin
        $display("FAILED const_3 default");
        top.pass = 1'b0;
      end
    endcase
  end
endmodule

// All delays should be delay.
module var_1 (output out, input in);
  time delay = 200;
  assign #(delay) out = (in === 1'bx) ? 1'bz : ~in;

  always @(out) begin
    case (out)
      1'b0: if ($time - top.change != delay) begin
        $display("Failed var_1 fall");
        top.pass = 1'b0;
      end
      1'b1: if ($time - top.change != delay) begin
        $display("Failed var_1 rise");
        top.pass = 1'b0;
      end
      1'bz: if ($time - top.change != delay) begin
        $display("Failed var_1 high-Z");
        top.pass = 1'b0;
      end
      default: begin
        $display("FAILED var_1 default");
        top.pass = 1'b0;
      end
    endcase
  end
endmodule

/*
 * We do not currently support calculating the decay time from the
 * variable rise and fall times. The compiler will print a message
 * and assert in the code generator.
 *
 * We need an a and b version to check both ways.
 *
// Decay should be the minimum of rise and fall delay.
module var_2 (output out, input in);
  time delayr = 100;
  time delayf = 200;
  assign #(delayr, delayf) out = ~in;

  function automatic real min_real(real a, real b);
    min_real = a < b ? a : b;
  endfunction

  always @(out) begin
    case (out)
      1'b0: if ($time - top.change != delayf) begin
        $display("Failed var_2 fall");
        top.pass = 1'b0;
      end
      1'b1: if ($time - top.change != delayr) begin
        $display("Failed var_2 rise");
        top.pass = 1'b0;
      end
      1'bz: if ($time - top.change != min_real(delayf, delayr)) begin
        $display("Failed var_2 high-Z");
        top.pass = 1'b0;
      end
      default: begin
        $display("FAILED var_2 default");
        top.pass = 1'b0;
      end
    endcase
  end
endmodule
*/

// All delays as given.
module var_3 (output out, input in);
  time delayr = 100;
  time delayf = 200;
  time delayd = 300;
  assign #(delayr, delayf, delayd) out = (in === 1'bx) ? 1'bz : ~in;

  always @(out) begin
    case (out)
      1'b0: if ($time - top.change != delayf) begin
        $display("Failed var_3 fall");
        top.pass = 1'b0;
      end
      1'b1: if ($time - top.change != delayr) begin
        $display("Failed var_3 rise");
        top.pass = 1'b0;
      end
      1'bz: if ($time - top.change != delayd) begin
        $display("Failed var_3 high-Z");
        top.pass = 1'b0;
      end
      default: begin
        $display("FAILED var_3 default");
        top.pass = 1'b0;
      end
    endcase
  end
endmodule
