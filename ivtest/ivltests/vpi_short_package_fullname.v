// Check VPI full names for objects in a package with a one-character name.

package p;
  integer value;
  reg [7:0] bits;
  real real_value;
  parameter INTEGER_PARAM = 1;
  parameter REAL_PARAM = 1.5;
  parameter TEXT_PARAM = "text";
  reg [7:0] words[2:3];
  event changed;

  task show;
    integer local_value;
    $vpi_tree(local_value);
  endtask
endpackage

package pp;
  integer value;
endpackage

module m;
  integer value;
endmodule

module test;
  integer value;

  initial begin
    $vpi_tree(p::value, p::bits, p::real_value);
    $vpi_tree(p::INTEGER_PARAM, p::REAL_PARAM, p::TEXT_PARAM);
    $vpi_tree(p::words, p::changed);
    p::show();
    $vpi_tree(pp::value, m.value, value);
  end
endmodule
