module main;

   reg [31:0] bytes;

   initial begin
      bytes = "\101\102\103\n";
      $write("bytes=%h\n", bytes);
      $finish(0);
   end
endmodule // main
