// Check that it is possible to call void function from within a class method.
// Check this for functions defined outside the class as well as functions that
// are methods of the class or base class.

integer sum = 0;

function void f1(integer x);
  sum += x;
endfunction

class B;
  function void f2(integer x);
    sum += x;
  endfunction
endclass

class C extends B;

  function void f3(integer x);
    sum += x;
  endfunction

  task t;
    f1(10);
    f2(20);
    f3(30);
  endtask

endclass

module test;

  C c = new;

  initial begin
    c.t;
    c.f2(40);
    c.f3(50);
    if (sum === 150) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
