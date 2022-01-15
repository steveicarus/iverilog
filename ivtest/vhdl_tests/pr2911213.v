module top();

  reg [3:0] a;
  reg [4:0] b;
  wire [8:0] y;

  functest uut(a, b, y);

  initial begin
    a = 3'b101;
    b = 4'b0101;
    #1;
    if (y == 8'b10100101)
      $display("PASSED");
    else
      $display("FAILED y = %b", y);
  end

endmodule // top


module functest (
    operand_a,
    operand_b,

    result_y
    );

    input [3:0] operand_a;
    input [4:0] operand_b;
    output [8:0] result_y;

    function [8:0] concat_this;
        input [3:0] op_s;
        input [4:0] op_l;

        concat_this = {op_s, op_l};
    endfunction

    reg [8:0] result_y_wire;

always @ (operand_a or operand_b) begin
    result_y_wire = concat_this(operand_a, operand_b);
end

assign result_y = result_y_wire;

endmodule
