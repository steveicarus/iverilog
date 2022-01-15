`define my_macro(a,b)	localparam `` i``a``b``j = 8'h``a``b; \
\

module test();

`my_macro(0,1)

`my_macro( 2, 3)

`my_macro(  4  ,  5  )

reg failed = 0;

initial begin
  $display("%h", i01j);
  if (i01j !== 8'h01) failed = 1;
  $display("%h", i23j);
  if (i23j !== 8'h23) failed = 1;
  $display("%h", i45j);
  if (i45j !== 8'h45) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
