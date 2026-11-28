// Check that a function that is declared outside of the class can be
// called as a statement from a static method. A static method has no
// object, so it must not try to look the name up as "this.<name>".

int pf_count = 0;

function void pf();
  pf_count += 1;
endfunction

function int pg(int a);
  return a + 1;
endfunction

class C;
  static function void sf();
    pf();
  endfunction

  static function void sf2();
    void'(pg(1));
  endfunction

  static function int sf3();
    return pg(1);
  endfunction

  function void nf();
    pf();
  endfunction
endclass

module test;

  bit failed = 0;
  C c;

  initial begin
    c = new;
    C::sf();
    if (pf_count != 1) failed = 1;
    C::sf2();
    if (pf_count != 1) failed = 1;
    if (C::sf3() != 2) failed = 1;
    c.nf();
    if (pf_count != 2) failed = 1;
    if (failed) $display("FAILED");
    else $display("PASSED");
  end

endmodule
