`timescale 1ps/1ps
module overflow_source(input A, output Y);
  buf (Y,A);
endmodule
module overflow_load(input A, output Y);
  buf (Y,A);
endmodule
`timescale 1ns/1ns
module overflow_coarse(input A, output Y);
  buf (Y,A);
endmodule
`timescale 1ps/1ps
module test;
  reg a;
  wire n,q,c;
  integer errors;
  overflow_source src(a,n);
  overflow_load sink(n,q);
  overflow_coarse coarse(n,c);
  initial begin
    a=0; errors=0;
    #1;
    $sdf_annotate("ivltests/sdf_interconnect_increment_overflow_absolute.sdf",test);
    if(!$check_increment_overflow)errors=errors+1;
    $sdf_annotate("ivltests/sdf_interconnect_increment_overflow_add.sdf",test);
    // A valid first slot must not be committed when the second overflows.
    if(!$check_increment_overflow)errors=errors+1;
    if(errors==0)$display("PASSED");
  end
endmodule
