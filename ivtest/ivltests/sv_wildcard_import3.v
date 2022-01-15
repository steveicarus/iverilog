package my_package;

parameter  p1 = 1;
localparam p2 = 2;

typedef logic [3:0] word;

word v = 0;

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

parameter  p1 = 'bx;
localparam p2 = 'bx;

typedef logic [7:0] word;

word v = 8'bx;

event e;

word my_v = 1;

initial begin
  #1 ->my_package::e;
end

initial begin
  @(my_package::e);
  #1 my_v = 8'bx;
  #1 ->e;
end

initial begin:my_block
  import my_package::*;
  // Because this is a new scope, we should use the
  // imported versions of p1, p2, e, v, f, and h.
  @e v = my_v;
  h(f(1));
  if (p1 === 1 && p2 === 2 && $bits(v) === 4 && v === 3)
    $display("PASSED");
  else
    $display("FAILED");
end

function word f(word g);
  f = g + 8'bx;
endfunction

task h(word i);
  v = v + i + 8'bx;
  $display(v);
endtask

endmodule
