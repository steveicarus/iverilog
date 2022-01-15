module elsif_test();

`define	DEFINED

integer i;

initial begin
  i = 0;

`ifdef DEFINED
  `ifdef DEFINED
    i = i + 1;
  `elsif DEFINED
    i = 100;
  `else
    i = 110;
  `endif
`elsif DEFINED
  `ifdef DEFINED
    i = 120;
  `elsif DEFINED
    i = 130;
  `else
    i = 140;
  `endif
`else
  `ifdef DEFINED
    i = 150;
  `elsif DEFINED
    i = 160;
  `else
    i = 170;
  `endif
`endif

`ifdef UNDEFINED
  `ifdef DEFINED
    i = 200;
  `elsif DEFINED
    i = 210;
  `else
    i = 220;
  `endif
`elsif DEFINED
  `ifdef UNDEFINED
    i = 230;
  `elsif DEFINED
    i = i + 1;
  `else
    i = 240;
  `endif
`else
  `ifdef UNDEFINED
    i = 250;
  `elsif DEFINED
    i = 260;
  `else
    i = 270;
  `endif
`endif

`ifdef UNDEFINED
  `ifdef UNDEFINED
    i = 300;
  `elsif UNDEFINED
    i = 310;
  `else
    i = 320;
  `endif
`elsif UNDEFINED
  `ifdef UNDEFINED
    i = 330;
  `elsif UNDEFINED
    i = 340;
  `else
    i = 350;
  `endif
`else
  `ifdef UNDEFINED
    i = 360;
  `elsif UNDEFINED
    i = 370;
  `else
    i = i + 1;
  `endif
`endif

  if (i == 3)
    $display("PASSED");
  else
    $display("Test FAILED: %d", i);
end

endmodule
