/* fopen2 - test $fopen and $fclose system tasks */

module fopen2;
   integer fp1, fp2, fp3, fp4;
   integer dfp;

   reg error;
   reg [31:0] foo;

   initial begin
      error = 0;
      fp1 = $fopen("work/fopen2.out1");
      checkfp(fp1);
      dfp = fp1|1;
      $fdisplay(dfp, "fp1=%d", fp1);

      fp2 = $fopen("work/fopen2.out2");
      checkfp(fp2);
      dfp = fp2|1;
      $fdisplay(dfp, "fp2=%d", fp2);

      fp3 = $fopen("work/fopen2.out3");
      checkfp(fp3);
      dfp = fp3|1;
      $fdisplay(dfp, "fp3=%d", fp3);

      $fclose(fp2);

      fp4 = $fopen("work/fopen2.out4");
      checkfp(fp4);
      dfp = fp4|1;
      $fdisplay(dfp, "fp4=%d", fp4);

      $fclose(fp1);
      $fclose(fp2);
      $fclose(fp3);
      $fclose(fp4);
      if(error == 0)
          $display("PASSED");

   end // initial begin

   task checkfp;
      input [31:0] fp;
      begin
	 if(fp != 2 && fp != 4 && fp != 8 && fp != 16 && fp != 32 && fp != 64) begin
	    $display("FAILED fopen fp=%d", fp);
        error = 1;
	 end
      end
   endtask // checkfp

endmodule
