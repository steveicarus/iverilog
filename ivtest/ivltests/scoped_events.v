// Copyright 2008, Martin Whitaker.
// This file may be freely copied for any purpose.

module sub_module();

generate
  genvar i;
  for (i = 0; i < 4; i = i + 1) begin:gen_block
    localparam l = i + 1;
    event trigger;

    always @trigger $display("generate block %0d triggered", l);
  end
endgenerate

initial begin:my_block
  parameter p = 0;
  localparam l = p + 1;
  event trigger;
  @trigger $display("block %0d triggered", l);
end

task my_task;
  parameter p = 0;
  localparam l = p + 1;
  event trigger;
  @trigger $display("task %0d triggered", l);
endtask

initial my_task;

endmodule


module top_module();

sub_module sub();

defparam sub.my_block.p = 4;
defparam sub.my_task.p = 5;

initial begin
  #1 ->sub.gen_block[0].trigger;
  #1 ->sub.gen_block[1].trigger;
  #1 ->sub.gen_block[2].trigger;
  #1 ->sub.gen_block[3].trigger;
  #1 ->sub.my_block.trigger;
  #1 ->sub.my_task.trigger;
  #1 $finish(0);
end

endmodule
