// This tests system functions operationg on packed arrays
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2012 by Iztok Jeras.

module test ();

   // parameters for array sizes
   localparam WA = 4;
   localparam WB = 6;
   localparam WC = 8;

   function int wdt (input int i);
     wdt = 2 + 2*i;
   endfunction

   // 2D packed arrays
   logic [WA-1:0] [WB-1:0] [WC-1:0] abg;  // big    endian array
   logic [0:WA-1] [0:WB-1] [0:WC-1] alt;  // little endian array

   // error counter
   bit err = 0;

   // indexing variable
   int i;

   initial begin
      // big endian

      // full array
      if ($dimensions(abg) != 3)        begin $display("FAILED -- $dimensions(abg) = %0d", $dimensions(abg)); err=1; end;
      if ($bits      (abg) != WA*WB*WC) begin $display("FAILED -- $bits      (abg) = %0d", $bits      (abg)); err=1; end;
      for (i=1; i<=3; i=i+1) begin
         if ($left     (abg     , i) != wdt(i  )-1) begin $display("FAILED -- $left     (abg     , %0d) = %0d", i, $left     (abg     , i)); err=1; end;
         if ($right    (abg     , i) != 0         ) begin $display("FAILED -- $right    (abg     , %0d) = %0d", i, $right    (abg     , i)); err=1; end;
         if ($low      (abg     , i) != 0         ) begin $display("FAILED -- $low      (abg     , %0d) = %0d", i, $low      (abg     , i)); err=1; end;
         if ($high     (abg     , i) != wdt(i  )-1) begin $display("FAILED -- $high     (abg     , %0d) = %0d", i, $high     (abg     , i)); err=1; end;
         if ($increment(abg     , i) != 1         ) begin $display("FAILED -- $increment(abg     , %0d) = %0d", i, $increment(abg     , i)); err=1; end;
         if ($size     (abg     , i) != wdt(i  )  ) begin $display("FAILED -- $size     (abg     , %0d) = %0d", i, $size     (abg     , i)); err=1; end;
      end
      // half array
      if ($dimensions(abg[1:0]) != 3)       begin $display("FAILED -- $dimensions(abg[1:0]) = %0d", $dimensions(abg[1:0])); err=1; end;
      if ($bits      (abg[1:0]) != 2*WB*WC) begin $display("FAILED -- $bits      (abg[1:0]) = %0d", $bits      (abg[1:0])); err=1; end;
      for (i=1; i<=3; i=i+1) begin
         if ($left     (abg[1:0], i) != wdt(i  )-1) begin $display("FAILED -- $left     (abg[1:0], %0d) = %0d", i, $left     (abg[1:0], i)); err=1; end;
         if ($right    (abg[1:0], i) != 0         ) begin $display("FAILED -- $right    (abg[1:0], %0d) = %0d", i, $right    (abg[1:0], i)); err=1; end;
         if ($low      (abg[1:0], i) != 0         ) begin $display("FAILED -- $low      (abg[1:0], %0d) = %0d", i, $low      (abg[1:0], i)); err=1; end;
         if ($high     (abg[1:0], i) != wdt(i  )-1) begin $display("FAILED -- $high     (abg[1:0], %0d) = %0d", i, $high     (abg[1:0], i)); err=1; end;
         if ($increment(abg[1:0], i) != 1         ) begin $display("FAILED -- $increment(abg[1:0], %0d) = %0d", i, $increment(abg[1:0], i)); err=1; end;
         if ($size     (abg[1:0], i) != wdt(i  )  ) begin $display("FAILED -- $size     (abg[1:0], %0d) = %0d", i, $size     (abg[1:0], i)); err=1; end;
      end
      // single array element
      if ($dimensions(abg[0]) != 2)     begin $display("FAILED -- $dimensions(abg[0]) = %0d", $dimensions(abg[0])); err=1; end;
      if ($bits      (abg[0]) != WB*WC) begin $display("FAILED -- $bits      (abg[0]) = %0d", $bits      (abg[0])); err=1; end;
      for (i=1; i<=2; i=i+1) begin
         if ($left     (abg[0]  , i) != wdt(i+1)-1) begin $display("FAILED -- $left     (abg[0]  , %0d) = %0d", i, $left     (abg[0]  , i)); err=1; end;
         if ($right    (abg[0]  , i) != 0         ) begin $display("FAILED -- $right    (abg[0]  , %0d) = %0d", i, $right    (abg[0]  , i)); err=1; end;
         if ($low      (abg[0]  , i) != 0         ) begin $display("FAILED -- $low      (abg[0]  , %0d) = %0d", i, $low      (abg[0]  , i)); err=1; end;
         if ($high     (abg[0]  , i) != wdt(i+1)-1) begin $display("FAILED -- $high     (abg[0]  , %0d) = %0d", i, $high     (abg[0]  , i)); err=1; end;
         if ($increment(abg[0]  , i) != 1         ) begin $display("FAILED -- $increment(abg[0]  , %0d) = %0d", i, $increment(abg[0]  , i)); err=1; end;
         if ($size     (abg[0]  , i) != wdt(i+1)  ) begin $display("FAILED -- $size     (abg[0]  , %0d) = %0d", i, $size     (abg[0]  , i)); err=1; end;
      end

      // little endian

      // full array
      if ($dimensions(alt) != 3)        begin $display("FAILED -- $dimensions(alt) = %0d", $dimensions(alt)); err=1; end;
      if ($bits      (alt) != WA*WB*WC) begin $display("FAILED -- $bits      (alt) = %0d", $bits      (alt)); err=1; end;
      for (i=1; i<=3; i=i+1) begin
         if ($left     (alt     , i) != 0         ) begin $display("FAILED -- $left     (alt     , %0d) = %0d", i, $left     (alt     , i)); err=1; end;
         if ($right    (alt     , i) != wdt(i  )-1) begin $display("FAILED -- $right    (alt     , %0d) = %0d", i, $right    (alt     , i)); err=1; end;
         if ($low      (alt     , i) != 0         ) begin $display("FAILED -- $low      (alt     , %0d) = %0d", i, $low      (alt     , i)); err=1; end;
         if ($high     (alt     , i) != wdt(i  )-1) begin $display("FAILED -- $high     (alt     , %0d) = %0d", i, $high     (alt     , i)); err=1; end;
         if ($increment(alt     , i) != -1        ) begin $display("FAILED -- $increment(alt     , %0d) = %0d", i, $increment(alt     , i)); err=1; end;
         if ($size     (alt     , i) != wdt(i  )  ) begin $display("FAILED -- $size     (alt     , %0d) = %0d", i, $size     (alt     , i)); err=1; end;
      end
      // half array
      if ($dimensions(alt[0:1]) != 3)       begin $display("FAILED -- $dimensions(alt[0:1]) = %0d", $dimensions(alt[0:1])); err=1; end;
      if ($bits      (alt[0:1]) != 2*WB*WC) begin $display("FAILED -- $bits      (alt[0:1]) = %0d", $bits      (alt[0:1])); err=1; end;
      for (i=1; i<=3; i=i+1) begin
         if ($left     (alt[0:1], i) != 0         ) begin $display("FAILED -- $left     (alt[0:1], %0d) = %0d", i, $left     (alt[0:1], i)); err=1; end;
         if ($right    (alt[0:1], i) != wdt(i  )-1) begin $display("FAILED -- $right    (alt[0:1], %0d) = %0d", i, $right    (alt[0:1], i)); err=1; end;
         if ($low      (alt[0:1], i) != 0         ) begin $display("FAILED -- $low      (alt[0:1], %0d) = %0d", i, $low      (alt[0:1], i)); err=1; end;
         if ($high     (alt[0:1], i) != wdt(i  )-1) begin $display("FAILED -- $high     (alt[0:1], %0d) = %0d", i, $high     (alt[0:1], i)); err=1; end;
         if ($increment(alt[0:1], i) != -1        ) begin $display("FAILED -- $increment(alt[0:1], %0d) = %0d", i, $increment(alt[0:1], i)); err=1; end;
         if ($size     (alt[0:1], i) != wdt(i  )  ) begin $display("FAILED -- $size     (alt[0:1], %0d) = %0d", i, $size     (alt[0:1], i)); err=1; end;
      end
      // single array element
      if ($dimensions(alt[0]) != 2)     begin $display("FAILED -- $dimensions(alt) = %0d", $dimensions(alt)); err=1; end;
      if ($bits      (alt[0]) != WB*WC) begin $display("FAILED -- $bits      (alt) = %0d", $bits      (alt)); err=1; end;
      for (i=1; i<=2; i=i+1) begin
         if ($left     (alt[0]  , i) != 0         ) begin $display("FAILED -- $left     (alt[0]  , %0d) = %0d", i, $left     (alt[0]  , i)); err=1; end;
         if ($right    (alt[0]  , i) != wdt(i+1)-1) begin $display("FAILED -- $right    (alt[0]  , %0d) = %0d", i, $right    (alt[0]  , i)); err=1; end;
         if ($low      (alt[0]  , i) != 0         ) begin $display("FAILED -- $low      (alt[0]  , %0d) = %0d", i, $low      (alt[0]  , i)); err=1; end;
         if ($high     (alt[0]  , i) != wdt(i+1)-1) begin $display("FAILED -- $high     (alt[0]  , %0d) = %0d", i, $high     (alt[0]  , i)); err=1; end;
         if ($increment(alt[0]  , i) != -1        ) begin $display("FAILED -- $increment(alt[0]  , %0d) = %0d", i, $increment(alt[0]  , i)); err=1; end;
         if ($size     (alt[0]  , i) != wdt(i+1)  ) begin $display("FAILED -- $size     (alt[0]  , %0d) = %0d", i, $size     (alt[0]  , i)); err=1; end;
      end

      if (!err) $display("PASSED");
   end

endmodule // test
