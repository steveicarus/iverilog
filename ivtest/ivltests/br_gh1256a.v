module test();

typedef struct packed {
  logic [3:0] a;
} inner_t;

typedef struct packed {
  inner_t [1:0][3:0] fields;
} outer_t;

outer_t var1;
outer_t var2;

initial begin
  var1 = 32'h12345678;
  var2.fields = var1.fields;
  $display("%h", var2);
  if (var2 === 32'h12345678)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
