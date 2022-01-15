module top;
  integer chr, fd, code;
  reg [14*8:1] str;

  initial begin
    // Put a string into the file.
    fd = $fopen("work/test.txt", "w");
    if (fd == 0) begin
      $display("Failed to open test file for writing!");
      $finish;
    end
    $fdisplay(fd, "Hello World!");
    $fclose(fd);

    // Now read it back and verify that $ungetc() and other things work.
    fd = $fopen("work/test.txt", "r");
    if (fd == 0) begin
      $display("Failed to open test file for reading!");
      $finish;
    end

    chr = $fgetc(fd);
    if (chr != "H") begin
      $display("Failed first character read!");
      $finish;
    end

    code = $ungetc(chr, fd);
    if (code == -1) begin
      $display("Failed to ungetc() character!");
      $finish;
    end

    chr = $fgetc(fd);
    if (chr != "H") begin
      $display("Failed first character reread!");
      $finish;
    end

    code = $ungetc(chr, fd);
    if (code == -1) begin
      $display("Failed to ungetc() character (2)!");
      $finish;
    end

    code = $fgets(str, fd);
    if (code == 0) begin
      $display("Failed to read characters!");
      $finish;
    end
    if (str[13*8:9] != "Hello World!") begin
      $display("Read wrong characters!");
      $finish;
    end

    $fclose(fd);

    $display("PASSED");
  end
endmodule
