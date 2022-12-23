// Check that it is possible to access a package scoped identifier of the same
// name of a class property inside a class method

package P;
  int x = 10;
endpackage

module test;

  class C;
    int x = 0;

    task check;
      if (P::x === 10) begin
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
