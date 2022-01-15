module test();

typedef bit     [63:0] bit64;
typedef logic   [63:0] vec64;

byte            byte_array [];
bit     [15:0]  bit_array  [];
logic   [31:0]  vec_array  [];
real            real_array [];

bit64           bit_result;
vec64           vec_result;

reg failed = 0;

initial begin
  byte_array = new [8];
  foreach (byte_array[i]) byte_array[i] = i*16 + i;

  bit_result = bit64'(byte_array);
  $display("%h", bit_result);
  if (bit_result !== 64'h0011223344556677) failed = 1;

  vec_result = vec64'(byte_array);
  $display("%h", vec_result);
  if (vec_result !== 64'h0011223344556677) failed = 1;

  bit_array = new [4];
  foreach (bit_array[i]) bit_array[i] = i*4096 + i*256 + i*16 + i;

  bit_result = bit64'(bit_array);
  $display("%h", bit_result);
  if (bit_result !== 64'h0000111122223333) failed = 1;

  vec_result = vec64'(bit_array);
  $display("%h", vec_result);
  if (vec_result !== 64'h0000111122223333) failed = 1;

  vec_array = new [2];
  vec_array[0] = 32'b01xz_0001_0010_0011_0100_0101_0110_0111;
  vec_array[1] = 32'b1000_1001_1010_1011_1100_1101_1110_1111;

  bit_result = bit64'(vec_array);
  $display("%h", bit_result);
  if (bit_result !== 64'b0100_0001_0010_0011_0100_0101_0110_0111_1000_1001_1010_1011_1100_1101_1110_1111) failed = 1;

  vec_result = vec64'(vec_array);
  $display("%h", vec_result);
  if (vec_result !== 64'b01xz_0001_0010_0011_0100_0101_0110_0111_1000_1001_1010_1011_1100_1101_1110_1111) failed = 1;

  real_array = new [1];
  real_array[0] = 1.2345678;

  bit_result = bit64'(real_array);
  $display("%h", bit_result);
  if (bit_result !== $realtobits(1.2345678)) failed = 1;

  vec_result = vec64'(real_array);
  $display("%h", vec_result);
  if (vec_result !== $realtobits(1.2345678)) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
