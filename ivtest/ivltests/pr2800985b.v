/*
 * Check $ferror() compile time errors.
 */
module top;
  integer errno, fd;
  reg [639:0] result;
  reg [63:0] sresult;

  initial begin
    fd = 0;
    errno = $ferror("string", result);  // Invalid first argument.
    errno = $ferror(fd);                // Missing second argument.
    errno = $ferror(fd, "string");      // Invalid second argument.
    errno = $ferror(fd, sresult);       // Second argument is too small.
    errno = $ferror(fd, result, "xx");  // Extra arguments.
  end
endmodule
