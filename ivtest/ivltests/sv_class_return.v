// Check that functions returning a class object are supported

module test;

  class C;
    int i;
    task t;
      if (i == 10) begin
        $display("PASSED");
      end else begin
        $display("FAILED");
      end
    endtask
  endclass

  function C f;
    C c;
    c = new;
    c.i = 10;
    return c;
  endfunction

  initial begin
    C c;
    c = f();
    c.t;
  end

endmodule
