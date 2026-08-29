// Check that $unit:: does not allow a forward variable reference.

module test;

  initial $display("%0d", $unit::value);

endmodule

integer value;
