// Copyright (c) 2026 Jerry Zhong
// Check integer conversion through signal reads and a file descriptor.

module test;
  reg [31:0] word;
  reg [7:0] small_unsigned;
  reg signed [7:0] small_signed;
  reg [39:0] wide_unsigned;
  reg signed [39:0] wide_signed;
  reg [8*32-1:0] line;
  integer fd, count;

  initial begin
    word = 0; $check_vector4_integer(word);
    word = 32'hffffffff; $check_vector4_integer(word);
    word = 32'h80000000; $check_vector4_integer(word);
    word = 32'h7fffffff; $check_vector4_integer(word);
    small_unsigned = 8'h80; $check_vector4_integer(small_unsigned);
    small_signed = 8'h80; $check_vector4_integer(small_signed);
    wide_unsigned = 40'hab12345678; $check_vector4_integer(wide_unsigned);
    wide_signed = 40'hff80000001; $check_vector4_integer(wide_signed);
    word = 32'hx00z00a5; $check_vector4_integer(word);
    small_signed = 8'b1xz00101; $check_vector4_integer(small_signed);
    small_signed = 8'bxz000101; $check_vector4_integer(small_signed);

    fd = $fopen("work/vpi_vector4_integer.txt", "w");
    if (!fd) begin
      $display("FAILED opening output file");
      $finish;
    end
    $fdisplay(fd, "conversion");
    $fclose(fd);
    fd = $fopen("work/vpi_vector4_integer.txt", "r");
    if (!fd) begin
      $display("FAILED opening input file");
      $finish;
    end
    count = $fgets(line, fd);
    $fclose(fd);
    if (count != 11 || line != "conversion\n") begin
      $display("FAILED file readback");
      $finish;
    end
    $check_vector4_integer_done;
    $finish;
  end
endmodule
