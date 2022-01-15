// This tests end labes (test should pass compilation)
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2012 by Iztok Jeras.

module test ();

   // error counter
   bit err = 0;

   initial
   begin : dummy_label
      if (!err) $display("PASSED");
   end : dummy_label

endmodule : test
