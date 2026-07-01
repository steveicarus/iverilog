// Check that assignment patterns cannot reference automatic variables in
// procedural force statements.

module test;

  reg [3:0] result;

  task automatic t;
    input [3:0] value;
    begin
      force result = '{value[3], value[2], value[1], value[0]};
    end
  endtask

  initial begin
    t(4'ha);
  end

endmodule
