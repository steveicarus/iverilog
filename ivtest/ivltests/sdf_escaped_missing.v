`timescale 1ps/1ps
// A missing literal a.b must not select the existing hierarchy a.b.
module delay_cell(input A, output Z);
  buf (Z, A);
  specify (A => Z) = (1, 1); endspecify
endmodule
module nested(input A, output Z);
  delay_cell b(A, Z);
endmodule
module driver(input A, output Z);
  buf (Z, A);
endmodule
module receiver(input A, output Z);
  buf (Z, A);
endmodule
module top;
  reg A = 0;
  wire hierarchy_z, drive, Z;
  integer hierarchy_count = 0, count = 0;
  nested a(A, hierarchy_z);
  driver d(A, drive);
  receiver r(drive, Z);
  initial begin
    $sdf_annotate("ivltests/sdf_escaped_missing.sdf", top);
    #100 A = 1;
    #100 A = 0;
    #50;
    if (hierarchy_count != 2 || count != 2)
      $fatal(1, "FAIL missing-target counts %0d %0d", hierarchy_count, count);
    $display("PASSED");
    $finish(0);
  end
  always @(hierarchy_z) if ($time > 50) begin
    hierarchy_count = hierarchy_count + 1;
    if (!((hierarchy_z === 1'b1 && $time == 101) ||
          (hierarchy_z === 1'b0 && $time == 201)))
      $fatal(1, "FAIL missing literal changed hierarchy at %0t", $time);
  end
  always @(Z) if ($time > 50) begin
    count = count + 1;
    if (!((Z === 1'b1 && $time == 107) ||
          (Z === 1'b0 && $time == 209)))
      $fatal(1, "FAIL annotation after missing port at %0t", $time);
  end
endmodule
