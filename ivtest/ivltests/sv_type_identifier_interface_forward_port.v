// Check that a forward interface name can match a visible typedef at a port.

package p;
  typedef int IFACE;
endpackage

import p::*;

module dut(IFACE i_iface, output logic result);

  assign result = i_iface.value;

endmodule

interface IFACE;
  logic value;
endinterface

module test;

  IFACE i_iface();
  logic result;

  dut i_dut(i_iface, result);

  initial begin
    i_iface.value = 1'b1;
    #1;

    if (result === 1'b1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
