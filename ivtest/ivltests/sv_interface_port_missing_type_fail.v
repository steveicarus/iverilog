// This tests the diagnostic path for an interface-typed module port
// whose interface type name is not declared.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

module bus_user(
   missing_if.consumer bus
);
   initial $display("FAILED");
endmodule
