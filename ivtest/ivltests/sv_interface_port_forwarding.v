// This tests forwarding an interface-typed formal port to a child
// module while preserving the parent-facing modport view.
//
// This file is placed into the Public Domain, for any use, without
// warranty.

interface bus_if ();
   logic value;
   logic mirror;
   logic hidden;

   modport consumer(input value, output mirror);
endinterface

module test;
   bus_if bus();

   assign bus.value = 1'b1;
   parent dut(.bus(bus));

   initial begin
      #1;
      if (bus.mirror !== 1'b1) begin
         $display("FAILED");
         $finish;
      end
      $display("PASSED");
   end
endmodule

module parent(
   bus_if.consumer bus
);
   child u_child(.bus(bus));
endmodule

module child(
   bus_if bus
);
   assign bus.mirror = bus.value;
endmodule
