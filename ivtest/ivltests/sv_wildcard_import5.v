package my_package1;

parameter  p1 = 1;
localparam p2 = 2;

typedef logic [1:0] word;

word v;

event e;

function word f(word g);
  f = g + 1;
endfunction

task h(word i);
  v = v + i;
  $display(v);
endtask

endpackage

package my_package2;

parameter  p1 = 1;
localparam p2 = 2;

typedef logic [1:0] word;

word v;

event e;

function word f(word g);
  f = g + 1;
endfunction

task h(word i);
  v = v + i;
  $display(v);
endtask

endpackage

module test();

import my_package1::*;
import my_package2::*;

word my_v;

initial begin
  @(e) v = p1 + p2;
  h(f(1));
end

endmodule
