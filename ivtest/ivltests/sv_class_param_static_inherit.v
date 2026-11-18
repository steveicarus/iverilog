// Check that a class scope also names the parameters, typedefs, static
// properties and static methods inherited from a base class, and calling
// static methods that are defined outside of the class (extern).

class B #(int BP = 3, type BT = int);
  localparam int BL = BP + 1;
  typedef BT bt_t;
  static int bcount = 100;

  static function int bsf();
    return BP;
  endfunction

  extern static function int ext(int a);
  extern static task etask(int a);
endclass

function int B::ext(int a);
  return a + BP;
endfunction

task B::etask(int a);
  bcount += a;
endtask

class D #(int DP = 2) extends B#(DP * 2);
  static function int dsf();
    return DP;
  endfunction
endclass

class E extends D#(5);
endclass

module test;

  bit failed = 0;
  D#(3)::bt_t v = 1;

  initial begin
    // D#(2) is based on B#(4), D#(5) on B#(10).
    if (D#(2)::BP != 4 || D::BP != 4 || E::BP != 10) failed = 1;
    if (D#(2)::BL != 5 || E::BL != 11) failed = 1;
    if (D#(2)::bsf() != 4 || E::bsf() != 10 || D#(2)::dsf() != 2) failed = 1;
    if (D#(3)::ext(1) != 7 || E::ext(1) != 11) failed = 1;
    if ($bits(v) != 32) failed = 1;

    // Each base specialization has its own static properties.
    D#(7)::bcount = 5;
    D::bcount++;
    E::etask(3);
    D#(7)::etask(1);
    if (D#(7)::bcount != 6 || D::bcount != 101 || E::bcount != 103) failed = 1;
    if (B::bcount != 100 || B#(14)::bcount != 6) failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end

endmodule
