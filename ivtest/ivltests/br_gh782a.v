module test;

initial begin
  $display("File %s line %0d", `__FILE__, `__LINE__);
`include "ivltests/br_gh782a.vi"  // single line comment
  $display("File %s line %0d", `__FILE__, `__LINE__);
`include "ivltests/br_gh782a.vi"  /* another single line comment */
  $display("File %s line %0d", `__FILE__, `__LINE__);
`include "ivltests/br_gh782a.vi"  /* multi-line
                                     comment */
  $display("File %s line %0d", `__FILE__, `__LINE__);
`include "ivltests/br_gh782a.vi"  /* single */ /* and
                                     multi-line comment */
  $display("File %s line %0d", `__FILE__, `__LINE__);
end

endmodule
