// Check classes that extend a specialization of a parameterized class, a
// parameterized class that extends another one using its own parameters, and
// virtual methods in a parameterized class.

class Base #(int W = 4);
  logic [W-1:0] data;
  int start;

  function new(int x);
    start = x + W;
  endfunction

  virtual function int size();
    return W;
  endfunction

  virtual function string name();
    return "Base";
  endfunction
endclass

class Byte extends Base #(8);
  function new();
    super.new(1);
  endfunction

  virtual function string name();
    return "Byte";
  endfunction
endclass

class Words #(int N = 2) extends Base #(N * 16);
  function new();
    super.new(N);
  endfunction

  virtual function int size();
    return super.size() + N;
  endfunction
endclass

module test;

  Base #(8) b8;
  Base #(48) b48;
  Byte by;
  Words w2;
  Words #(3) w3;
  bit failed = 0;

  initial begin
    by = new;
    w2 = new;
    w3 = new;

    b8 = by;
    if (b8.name() != "Byte" || b8.size() != 8 || b8.start != 9) failed = 1;
    if ($bits(by.data) != 8) failed = 1;

    if (w2.size() != 34 || w2.start != 34 || $bits(w2.data) != 32) failed = 1;

    b48 = w3;
    if (b48.size() != 51 || b48.name() != "Base" || b48.start != 51)
      failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end

endmodule
