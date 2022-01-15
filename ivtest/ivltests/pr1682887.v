// pr1682887

module tb();
   task mytask;
      begin end
   endtask

   initial mytask();
   initial #1 $display("PASSED");
endmodule
