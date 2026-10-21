// Check that delete() empties a dynamic array stored in a class property.

module test;

  class C;
    int data[];
  endclass

  C c;

  initial begin
    c = new;
    c.data = new[1];
    c.data.delete();

    if (c.data.size() !== 0) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule
