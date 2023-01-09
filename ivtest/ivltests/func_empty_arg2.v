// Check that it is possible to call functins with empty arguments if they have
// default values. Check that this works if the function call is in a module
// port binding expression.

module M (input [31:0] x, input [31:0] y);

  initial begin
    #1
    if (x === 623 && y == 624) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule

module test;

  function [31:0] f(reg [31:0] a, reg [31:0] b = 2, reg [31:0] c = 3);
    return a * 100 + b * 10 + c;
  endfunction

  M i_m (
    .x(f(6, )),
    .y(f(6, , 4))
  );

endmodule
