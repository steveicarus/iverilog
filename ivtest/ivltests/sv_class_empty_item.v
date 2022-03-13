// Check that empty item declarations are supported for classes

module test;

class C;
  ;
  int x;;

  task test;
    $display("PASSED");
  endtask;
  ;
endclass

  C c;

  initial begin
    c = new;
    c.test;
  end

endmodule
