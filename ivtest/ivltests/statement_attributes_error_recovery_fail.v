// Check that attributes do not cause crashes during statement error recovery.

module test;

  reg values [0:1];

  initial begin
    (* keep = 1 *) for (+) ; // Error: Invalid for loop header.
    (* keep = 1 *) while (+) ; // Error: Invalid while loop condition.
    (* keep = 1 *) do ; while (+); // Error: Invalid do/while loop condition.
    (* keep = 1 *) foreach (values[1]) ; // Error: Invalid foreach index.
    (* keep = 1 *) if (+) ; // Error: Invalid if condition.
    (* keep = 1 *) @(posedge) ; // Error: Missing event expression.
  end

endmodule
