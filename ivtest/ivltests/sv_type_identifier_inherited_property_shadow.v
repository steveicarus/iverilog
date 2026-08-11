// Check that an inherited property hides a compilation-unit type.

typedef logic [2:0] T;

class Base;
  logic [7:0] T;
endclass

class Derived extends Base;
  function integer width;
    width = $bits(T);
  endfunction

  function integer slice_width;
    slice_width = $bits(T[3:0]);
  endfunction
endclass

module test;

  Derived object;

  initial begin
    object = new;

    if (object.width() !== 8) begin
      $display("FAILED: expected 8, got %0d", object.width());
    end else if (object.slice_width() !== 4) begin
      $display("FAILED: expected 4, got %0d", object.slice_width());
    end else begin
      $display("PASSED");
    end
  end

endmodule
