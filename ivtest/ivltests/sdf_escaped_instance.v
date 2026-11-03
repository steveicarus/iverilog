`timescale 1ps/1ps
// Independent expected delays: literal a.b = 3/5 ps; hierarchy a.b = 11/13 ps.
module delay_cell(input A, output Z);
  buf (Z, A);
  specify (A => Z) = (1, 1); endspecify
endmodule
module nested(input A, output Z);
  delay_cell b(A, Z);
endmodule
module top;
  reg A = 0;
  wire literal_z, hierarchy_z;
  integer literal_count = 0, hierarchy_count = 0, errors = 0;
  delay_cell \a.b (A, literal_z);
  nested a(A, hierarchy_z);
  initial begin
`ifdef SLASH_DIVIDER
    $sdf_annotate("ivltests/sdf_escaped_instance_slash.sdf", top);
`else
    $sdf_annotate("ivltests/sdf_escaped_instance.sdf", top);
`endif
    #100 A = 1;
    #100 A = 0;
    #50;
    if (literal_count != 2 || hierarchy_count != 2 || errors != 0)
      $fatal(1, "FAIL literal=%0d hierarchy=%0d errors=%0d", literal_count, hierarchy_count, errors);
    $display("PASSED");
    $finish;
  end
  always @(literal_z) if ($time > 50) begin
    literal_count = literal_count + 1;
    $display("literal %0t %b", $time, literal_z);
    if (!((literal_z === 1'b1 && $time == 103) ||
          (literal_z === 1'b0 && $time == 205))) errors = errors + 1;
  end
  always @(hierarchy_z) if ($time > 50) begin
    hierarchy_count = hierarchy_count + 1;
    $display("hierarchy %0t %b", $time, hierarchy_z);
    if (!((hierarchy_z === 1'b1 && $time == 111) ||
          (hierarchy_z === 1'b0 && $time == 213))) errors = errors + 1;
  end
endmodule
