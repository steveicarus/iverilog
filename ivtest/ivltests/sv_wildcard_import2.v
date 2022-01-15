package my_package;

parameter  p1 = 1;
localparam p2 = p1 + 'bx;

typedef logic [1:0] word;

word v = 2'bx;

event e;

function word f(word g);
  f = g + 2'bx;
endfunction

task h(word i);
  v = v + i + 2'bx;
  $display(v);
endtask

endpackage

module test();

import my_package::*;

parameter  p1 = 3;
localparam p2 = p1 + 2;

typedef logic [7:0] word;

word v = 0;

event e;

word my_v = 0;

initial begin
  #1 ->my_package::e;
end

initial begin
  @(my_package::e);
  my_v = p1;
  #1 ->e;
end

initial begin
  @e v = my_v;
  h(f(1));
  if (p2 === 5 && $bits(v) === 8 && v === 5)
    $display("PASSED");
  else
    $display("FAILED");
end

function word f(word g);
  f = g + 1;
endfunction

task h(word i);
  v = v + i;
  $display(v);
endtask

endmodule
