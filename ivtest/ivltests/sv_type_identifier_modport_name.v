// Check that a modport name can shadow a visible type identifier.

typedef int M;

interface type_id_modport_ifc;
  logic value;

  modport M(input value);
endinterface

module test;

  type_id_modport_ifc i_ifc();

  initial begin
    $display("PASSED");
  end

endmodule
