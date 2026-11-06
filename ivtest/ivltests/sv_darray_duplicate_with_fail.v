// Check that a dynamic array method call cannot have two with clauses.

module test;
  int data[], result;

  initial begin
    result = data.sum() with (item * 2) with (item * 3); // Error: A call allows only one with clause.
  end
endmodule
