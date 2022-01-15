module main;

   initial begin
      # 32 $display("PASSED");
      $finish;
   end

   // This delay is 'h1_00000010. The idea here is if the delay is
   // only treated as 32 bits anywhere in the processing, then the
   // high bits are truncated, and it becomes 16, which is less then
   // the 32 above and we fail.
   initial begin
      # 4294967312 $display("FAILED -- time=%d", $time);
      $finish;
   end
endmodule // main
