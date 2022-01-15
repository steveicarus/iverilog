 `define EOF -1

 module bar;
   integer line, rc, file, a, b, c;
   reg [8*256:1] str;

   initial begin
      file = $fopen ("ivltests/pr1687193.dat", "r");
      if (file == 0)
	$finish;
      for (line = 1; line <= 5; line = line + 1) begin
	 rc = $fgets (str, file);
	 if (rc == `EOF)
	   $finish;
	 rc = $sscanf (str, "%h %h %h\n", a, b, c);
	 $display ("\tLine %d matches %d args: %h %h %h", line, rc, a, b, c);
      end
      $fclose (file);
      $finish(0);
   end
 endmodule
