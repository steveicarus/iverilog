// Check that an inherited property hides a compilation-unit type.

typedef logic [2:0] T;

class Base;
  logic [7:0] T;
endclass

class Derived extends Base;
  function void set(input logic [7:0] value);
    T = value;
  endfunction

  function logic [7:0] get;
    return T;
  endfunction

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
    object.set(8'ha5);

    if (object.get() !== 8'ha5) begin
      $display("FAILED: expected a5, got %0h", object.get());
    end else if (object.width() !== 8) begin
      $display("FAILED: expected 8, got %0d", object.width());
    end else if (object.slice_width() !== 4) begin
      $display("FAILED: expected 4, got %0d", object.slice_width());
    end else begin
      $display("PASSED");
    end
  end

endmodule
