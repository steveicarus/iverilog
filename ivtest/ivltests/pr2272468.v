module sub();

task task1;

input [1023:0] a;
input [1023:0] b;

begin
  if (a + b > 1026'd2) $display(1);
end

endtask

initial task1(1, 2);

endmodule

module top();

generate
  genvar i;

  for (i = 0; i < 256; i = i + 1) begin:block
    sub sub();
  end
endgenerate

endmodule
