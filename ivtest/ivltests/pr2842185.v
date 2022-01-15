module pr2842185();

// check that dection of signal/genvar name collisions
// observes scope boundaries.

genvar i;

task MyTask;

integer i;

begin
  $display("PASSED");
end

endtask

initial MyTask;

endmodule
