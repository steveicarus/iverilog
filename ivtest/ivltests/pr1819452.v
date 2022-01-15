module top;
  reg [8*80-1:0] str;
  integer fd, pos, result;

  initial begin
    fd = $fopen("ivltests/pr1819452.txt","rb");
    result = $fgets(str, fd);
    while (!$feof(fd)) begin
      pos = $ftell(fd);
      $display("Found: %5s currently at byte %0d", str[8*10-1:8], pos);
      result = $fgets(str, fd);
    end

    result = $rewind(fd);
    result = $fgets(str, fd);
    pos = $ftell(fd);
    $display("Found: %5s currently at byte %0d", str[8*10-1:8], pos);

    result = $fseek(fd, 0, 0);
    result = $fgets(str, fd);
    pos = $ftell(fd);
    $display("Found: %5s currently at byte %0d", str[8*10-1:8], pos);

    result = $fseek(fd, -3, 2);
    result = $fgets(str, fd);
    pos = $ftell(fd);
    $display("Found: %5s currently at byte %0d", str[8*10-1:8], pos);

    result = $fseek(fd, -6, 1);
    result = $fgets(str, fd);
    pos = $ftell(fd);
    $display("Found: %5s currently at byte %0d", str[8*10-1:8], pos);

    result = $fseek(32'hffffffff, 0, 0);
    $display("Check fseek EOF = %0d", result);

    result = $ftell(32'hffffffff);
    $display("Check ftell EOF = %0d", result);

    result = $rewind(32'hffffffff);
    $display("Check rewind EOF = %0d", result);

    $fclose(fd);
  end
endmodule
