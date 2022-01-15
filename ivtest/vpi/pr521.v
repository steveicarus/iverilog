module pli_test;

wire [15:0]   a = 16'h4321;
wire [ 7:0]   b = a[15:8];

integer rc;

initial
 begin
   #1 /* Allow the continuous assignments above to settle. */ ;
   $display("Passing parameter to PLI routine: 0x%x",a[15:8]);
   rc = $pli_test(a[15:8]);
   $display("Passing parameter to PLI routine: 0x%x",b);
   rc = $pli_test(b);
 end

endmodule
