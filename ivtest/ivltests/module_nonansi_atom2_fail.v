// Check that it is an error to declare a non-ANSI module port with implicit
// packed dimensions if it is later redeclared as an atom2 typed variable. Even
// if the size of the packed dimensions matches that of the size of the atom2
// type.

module test(x);
  output [15:0] x;
  shortint x;

  initial begin
    $display("FAILED");
  end

endmodule
