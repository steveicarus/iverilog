// Check that a class restricted type parameter rejects non-class overrides.

class class_t;
endclass

module M #(
  parameter type class T = class_t
);
endmodule

module test;

  M #(
    .T(int)
  ) i_m();

  initial begin
    $display("FAILED");
  end

endmodule
