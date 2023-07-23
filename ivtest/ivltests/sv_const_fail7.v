// Check that binding a const variable to a module output port fails.

module M(
  output integer x
);
  assign x = 20;
endmodule

module test;

  const integer x = 10;

  M i_m (
    .x (x)
  );

  initial begin
    $display("FAILED");
  end

endmodule
