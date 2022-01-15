class my_class;
  task run_test();
    $display("PASSED");
  endtask
endclass

module test();

class extended_class extends my_class;
endclass

extended_class obj;

initial begin
  obj = new();
  obj.run_test();
end

endmodule
