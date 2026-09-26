// Check a class property case expression with a wider case item.

class C;
  bit [7:0] value;
endclass

module test;

  C c;

  initial begin
    c = new;
    c.value = 7;

    case (c.value)
      64'd7: $display("PASSED");
      default: $display("FAILED");
    endcase
  end

endmodule
