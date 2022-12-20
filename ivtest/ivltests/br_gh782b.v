/* comment */ `timescale /* comment */ // comment
/* comment */ 1          /* comment */ // comment
/* comment */ s          /* comment */ // comment
/* comment */ /          /* comment */ // comment
/* comment */ 100        /* comment */ // comment
/* comment */ ms         /* comment */ // comment

module t1;
  initial begin
    $display("File %s line %0d", `__FILE__, `__LINE__);
    $printtimescale;
  end
endmodule

`timescale 100 ms / 10 ms // single line comment

module t2;
  initial begin
    $display("File %s line %0d", `__FILE__, `__LINE__);
    $printtimescale;
  end
endmodule

`timescale 10us/1us /* another single line comment */

module t3;
  initial begin
    $display("File %s line %0d", `__FILE__, `__LINE__);
    $printtimescale;
  end
endmodule

`timescale 1ns/1ps /* multi-line
                      comment */

module t4;
  initial begin
    $display("File %s line %0d", `__FILE__, `__LINE__);
    $printtimescale;
  end
endmodule

`timescale 1ps/1fs /* single */ /* and
                      multi-line comment */

module t5;
  initial begin
    $display("File %s line %0d", `__FILE__, `__LINE__);
    $printtimescale;
  end
endmodule
