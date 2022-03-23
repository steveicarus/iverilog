// Check that it is possible to overwrite a type identifier declared in a higher
// level scope. Check that this works when the new type is declared in a class.

typedef logic [3:0] T;
T x;

module test;

  class C;
    typedef logic [7:0] T;
    T y;

    task t;
      y = 8'hff;
      if ($bits(x) == 4 && $bits(y) == 8 && y == 8'hff) begin
        $display("PASSED");
      end else begin
        $display("FAILED");
      end
    endtask
  endclass

  C c;

  initial begin
    c = new;
    c.t();
  end

endmodule
