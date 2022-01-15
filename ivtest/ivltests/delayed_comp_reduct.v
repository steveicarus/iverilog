module top;
  reg [4:0]cntr;
  wire done;
  wire allone;

  // A delayed comparison is only 1 bit wide. If this does not crash
  // the run time then the compiler is producing correct code.
  assign #1 done = cntr == 'd7;
  // The same for a reduction.
  assign #1 allone = &cntr;

  initial $display("PASSED");
endmodule
