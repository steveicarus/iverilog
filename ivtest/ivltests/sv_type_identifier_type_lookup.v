// Check that visible type identifiers can be used as declaration types.

package p;
  typedef logic [3:0] PT;
endpackage

package q;
  typedef logic [12:0] I;
endpackage

typedef logic [7:0] T;
typedef logic [6:0] L;
typedef logic [8:0] I;

module test;

  import p::*;

  localparam integer outer_type_width = $bits(L);
  typedef logic [10:0] L;
  localparam integer local_type_width = $bits(L);

  localparam integer outer_import_width = $bits(I);
  import q::I;
  localparam integer imported_type_width = $bits(I);

  reg failed;
  T value;
  T [1:0] packed_value;
  PT imported_value;
  PT [1:0] imported_packed_value;

  function T inc(input T arg);
    inc = arg + 8'd1;
  endfunction

  initial begin
    p::PT scoped_value;
    p::PT [1:0] scoped_packed_value;

    failed = 1'b0;

    if (outer_type_width != 7) begin
      $display("FAILED(%0d). Outer typedef width mismatch", `__LINE__);
      failed = 1'b1;
    end

    if (local_type_width != 11) begin
      $display("FAILED(%0d). Local typedef width mismatch", `__LINE__);
      failed = 1'b1;
    end

    if (outer_import_width != 9) begin
      $display("FAILED(%0d). Outer typedef before import width mismatch",
	       `__LINE__);
      failed = 1'b1;
    end

    if (imported_type_width != 13) begin
      $display("FAILED(%0d). Imported typedef width mismatch", `__LINE__);
      failed = 1'b1;
    end

    value = 8'h12;
    packed_value = 16'h3456;
    imported_value = 4'h5;
    imported_packed_value = 8'h3c;
    scoped_value = 4'ha;
    scoped_packed_value = 8'hbc;

    if ($bits(value) != 8 || value !== 8'h12) begin
      $display("FAILED(%0d). Typedef declaration mismatch", `__LINE__);
      failed = 1'b1;
    end

    if ($bits(packed_value) != 16 || packed_value !== 16'h3456) begin
      $display("FAILED(%0d). Typedef packed declaration mismatch", `__LINE__);
      failed = 1'b1;
    end

    if ($bits(imported_value) != 4 || imported_value !== 4'h5) begin
      $display("FAILED(%0d). Imported typedef declaration mismatch", `__LINE__);
      failed = 1'b1;
    end

    if ($bits(imported_packed_value) != 8 || imported_packed_value !== 8'h3c) begin
      $display("FAILED(%0d). Imported typedef packed declaration mismatch", `__LINE__);
      failed = 1'b1;
    end

    if ($bits(scoped_value) != 4 || scoped_value !== 4'ha) begin
      $display("FAILED(%0d). Package-scoped typedef declaration mismatch", `__LINE__);
      failed = 1'b1;
    end

    if ($bits(scoped_packed_value) != 8 || scoped_packed_value !== 8'hbc) begin
      $display("FAILED(%0d). Package-scoped typedef packed declaration mismatch", `__LINE__);
      failed = 1'b1;
    end

    if (inc(value) !== 8'h13) begin
      $display("FAILED(%0d). Typedef function type mismatch", `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
