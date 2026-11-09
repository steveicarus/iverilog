// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module renamed(\ref.pin ,\data\pin ,\enable.pin );
 input \ref.pin ,\data\pin ,\enable.pin ;reg \notify.pin ;initial \notify.pin =0;
 specify
  $setup(\data\pin  &&& \enable.pin  === 1'b1,negedge \ref.pin  &&& \enable.pin  === 1'b1,10,\notify.pin );
  $hold(negedge \ref.pin  &&& \enable.pin  === 1'b1,\data\pin  &&& \enable.pin  === 1'b1,10,\notify.pin );
 endspecify
endmodule

module tb;
  reg a,b,c;renamed renamed_instance(a,b,c);
  initial begin a=1;b=0;c=0;#20 b=1;#5 a=0;#3 b=0;#2 a=1;
    #60 c=1;#10 b=1;#5 a=0;#3 b=0;
  end
  initial begin
    #1;
    $sh_watch("tb.renamed_instance", "setup", "ref.pin", "negedge", "data\\pin", "any", 10.0, 10, "notify.pin", 1'b0, 1, 105);
    $sh_watch("tb.renamed_instance", "hold", "ref.pin", "negedge", "data\\pin", "any", 10.0, 10, "notify.pin", 1'b0, 1, 108);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
