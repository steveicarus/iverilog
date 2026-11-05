// Check that defparam cannot override body parameters after an empty #().

module M #();
  parameter P = 7;
endmodule

module test;
  M i_m();
  defparam i_m.P = 9; // Error: A body parameter after #() cannot be overridden.
endmodule
