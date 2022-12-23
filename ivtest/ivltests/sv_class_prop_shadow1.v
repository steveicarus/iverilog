// Check that class properties can be shadowed by a local symbol

module test;

  class C;
    int x = 0;

    task check;
      int x; // This should shadow the class property
      x = 10;
      if (this.x == 0 && x === 10) begin
        $display("PASSED");
      end else begin
        $display("FAILED");
      end
    endtask
  endclass

  initial begin
    C c;
    c = new;
    c.check;
  end

endmodule
