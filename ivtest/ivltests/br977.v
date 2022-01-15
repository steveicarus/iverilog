`define DECLAREINT(name, i) integer name=i

module foo();

`DECLAREINT(bar, 2);

initial begin
  if (bar === 2)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
