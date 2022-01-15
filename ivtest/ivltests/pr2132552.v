module pr2132552();

task test_task;

parameter depth = 16;
parameter width = 8;

reg [width-1:0] mem [depth-1:0];

integer i;

begin
  for (i = 0; i < depth; i = i + 1) begin
    mem[i] = i;
  end
  for (i = 0; i < depth; i = i + 1) begin
    $display("%0d", mem[i]);
  end
end

endtask

initial test_task;

endmodule
