module test_file;

     reg  [64:1] file_name;
     initial begin
         file_name = 64'h4242434442424344;
     end
     subtest1 subtest1(file_name);

endmodule

module subtest1(file_name);

     input       [64:1]  file_name;
     wire        [64:1]  file_name;
     integer     outfile;
     initial #0 begin
         $display ("Execution started.");
         $display ("%s",file_name);
// I don't know if the following line conforms to spec or not.
         outfile = $fopen({"work/",file_name});
         $display ("Execution finished.");
         $fdisplay (outfile, "Recorded data in %s",file_name);
         $fclose (outfile);
         $finish(0);
     end

endmodule
