// Check that discipline nature references can match visible type identifiers.

nature potential_nature;
  units = "V";
  access = POTENTIAL_ACCESS;
endnature

nature flow_nature;
  units = "A";
  access = FLOW_ACCESS;
endnature

typedef int potential_nature;
typedef int flow_nature;

discipline electrical;
  potential potential_nature;
  flow flow_nature;
  domain continuous;
enddiscipline

module test;
  initial begin
    $display("PASSED");
  end
endmodule
