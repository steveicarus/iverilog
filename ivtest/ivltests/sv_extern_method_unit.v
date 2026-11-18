// Check extern methods of a class declared in the compilation unit scope.

class counter;
  int count;
  extern function new();
  extern function int next(int step = 1);
  extern task reset();
endclass

function counter::new();
  count = 0;
endfunction

function int counter::next(int step = 1);
  count += step;
  return count;
endfunction

task counter::reset();
  #1 count = 0;
endtask

module test;
  counter c;
  initial begin
    c = new();
    if (c.next() !== 1 || c.next(5) !== 6) begin
      $display("FAILED: next");
      $finish;
    end
    c.reset();
    if (c.count !== 0) begin
      $display("FAILED: reset count=%0d", c.count);
      $finish;
    end
    $display("PASSED");
  end
endmodule
