// Check that package import and export items can name a type identifier.

package type_id_name_pkg;
  typedef logic [3:0] T;
endpackage

package type_id_name_export_pkg;
  import type_id_name_pkg::T;
  export type_id_name_pkg::T;
endpackage

module test;
  import type_id_name_export_pkg::T;

  reg failed;
  T value;

  initial begin
    failed = 1'b0;
    value = 4'ha;

    if ($bits(value) != 4 || value != 4'ha) begin
      $display("FAILED(%0d). Imported type mismatch", `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
