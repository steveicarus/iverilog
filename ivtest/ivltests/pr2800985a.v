/*
 * Do some run time checks with $ferror(). We can not count on the return
 * code or the strings to be the same on different machines so we can only
 * look for the existence of an error and call that good enough. We humans
 * can look at the full output to see if it is correct.
 */
module top;
  parameter work_file = "work/pr2800985.txt";
  reg pass;
  integer errno, bfd, vfd, mcd, res;
  reg [639:0] result, str;

  /*
   * Tasks to check and clear the error state.
   */
  task check_error;
    begin
      // Check that there was an error.
      if (errno == 0) begin
        $display("  FAILED: expected an error!");
        pass = 1'b0;
      end
      clear_error;
    end
  endtask

  task clear_error;
    begin
      // Clear the error state.
      res = $ftell(vfd);
      errno = $ferror(vfd, result);
      if (errno != 0) begin
        $display("Failed to clear error state (%0s)", result);
        $finish;
      end
    end
  endtask

  initial begin
    pass = 1'b1;
    vfd = $fopen(work_file, "w+");
    if (vfd == 0) begin
      errno = $ferror(vfd, result);
      $display("Failed to open required file %s (%0s).", work_file, result);
      $finish;
    end

    /*
     * $ferror() only takes a fd, so a valid MCD is not valid..
     */
    $display("Check a valid MCD.");
    errno = $ferror(1, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Basic $fopen() checking.
     */
    $display("Opening a file that does not exist.");
    bfd = $fopen("FileDoesNotExist", "r");
    $display("  $fopen returned: %h", bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

/*
 * These do not work when the user has root privileges, so we need to
 * just skip them.
 *
    $display("Opening a file that we should not be able to write to.");
    bfd = $fopen("/FileDoesNotExist", "w");
    $display("  $fopen returned: %h", bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    $display("Opening a file that we should not be able to write to (MCD).");
    mcd = $fopen("/FileDoesNotExist");
    $display("  $fopen returned: %h", mcd);
    errno = $ferror(mcd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
*/

    $display("Opening a directory we should not be able to write.");
    bfd = $fopen("/", "w");
    $display("  $fopen returned: %h", bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $fclose().
     */
    $display("Checking $fclose with fd 0 and -1.");
    bfd = 0;
    $fclose(bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    $fclose(bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $fdisplay(), assume the b/h/o version work the same.
     */
    $display("Checking $fdisplay with fd 0 and -1.");
    bfd = 0;
    $fdisplay(bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    $fdisplay(bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $fwrite(), assume the b/h/o version work the same.
     */
    $display("Checking $fwrite with fd 0 and -1.");
    bfd = 0;
    $fwrite(bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    $fwrite(bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $fstrobe(), assume the b/h/o version work the same.
     */
    $display("Checking $fstrobe with fd 0 and -1.");
    bfd = 0;
    $fstrobe(bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    $fstrobe(bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $fgetc().
     */
    $display("Checking $fgetc with fd 0 and -1.");
    bfd = 0;
    res = $fgetc(bfd);
    $display("  $fgetc returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    res = $fgetc(bfd);
    $display("  $fgetc returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;


    /*
     * Check $ungetc().
     */
    $display("Checking $ungetc with fd 0 and -1 and char = EOF.");
    bfd = 0;
    res = $ungetc(0, bfd);
    $display("  $ungetc returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    res = $ungetc(0, bfd);
    $display("  $ungetc returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    // This returns EOF (-1), but does not set errno.
    res = $ungetc(-1, vfd);
    $display("  $ungetc returned: %0d", res);
    errno = $ferror(vfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    if (res != -1 || errno != 0) begin
      $display("  Failed expected result (-1, 0), got (%0d, %0d).", res, errno);
      // It's OK if this returns a value in errno.
      if (res != -1) pass = 1'b0;
      clear_error;
    end

    /*
     * Check $fgets().
     */
    $display("Checking $fgets with fd 0 and -1.");
    bfd = 0;
    res = $fgets(str, bfd);
    $display("  $fgets returned: %0d, '%0s'", res, str);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    res = $fgets(str, bfd);
    $display("  $fgets returned: %0d, '%0s'", res, str);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $fscanf().
     */
    $display("Checking $fscanf with fd 0 and -1.");
    bfd = 0;
    res = $fscanf(bfd, "%s", str);
    $display("  $fscanf returned: %0d, '%0s'", res, str);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    res = $fscanf(bfd, "%s", str);
    $display("  $fscanf returned: %0d, '%0s'", res, str);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $fread().
     */
    $display("Checking $fread with fd 0 and -1.");
    bfd = 0;
    res = $fread(str, bfd);
    $display("  $fread returned: %0d, '%0s'", res, str);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    res = $fread(str, bfd);
    $display("  $fread returned: %0d, '%0s'", res, str);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $ftell().
     */
    $display("Checking $ftell with fd 0 and -1.");
    bfd = 0;
    res = $ftell(bfd);
    $display("  $ftell returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    res = $ftell(bfd);
    $display("  $ftell returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $fseek().
     */
    $display("Checking $fseek.");
    bfd = 0;
    res = $fseek(bfd, 0, 0);
    $display("  $fseek returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    res = $fseek(bfd, 0, 0);
    $display("  $fseek returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    // A valid fd, but an invalid operation.
    res = $fseek(vfd, 0, 4);
    $display("  $fseek returned: %0d", res);
    errno = $ferror(vfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $rewind().
     */
    $display("Checking $rewind with fd 0 and -1.");
    bfd = 0;
    res = $rewind(bfd);
    $display("  $rewind returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    res = $rewind(bfd);
    $display("  $rewind returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $fflush().
     */
    $display("Checking $fflush with fd 0 and -1.");
    bfd = 0;
    $fflush(bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    $fflush(bfd);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $feof().
     */
    $display("Checking $feof with fd 0 and -1.");
    bfd = 0;
    res = $feof(bfd);
    $display("  $feof returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    res = $feof(bfd);
    $display("  $feof returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;

    /*
     * Check $fputc() (Icarus specific).
     */
`ifdef __ICARUS__
    $display("Checking $fputc with fd 0 and -1.");
    bfd = 0;
    res = $fputc(0, bfd);
    $display("  $fputc returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
    bfd = -1;
    res = $fputc(0, bfd);
    $display("  $fputc returned: %0d", res);
    errno = $ferror(bfd, result);
    $display("  $ferror returned: %0d, '%0s'", errno, result);
    check_error;
`endif

    /*
     * Check that $fstrobe does not access a file after it is closed.
     */
    $display("Checking $fstrobe after $fclose.");
    res = $rewind(vfd);
    if (res != 0) begin
      $display("Failed to rewind file");
      $finish;
    end
    $fwrite(vfd, "test-");
    $fstrobe(vfd, "FAILED");
    $fclose(vfd);
    vfd = $fopen(work_file, "r");
    if (vfd == 0) begin
      errno = $ferror(vfd, result);
      $display("Failed to open required file %s (%0s).", work_file, result);
      $finish;
    end
    res = $fgets(str, vfd);
    if (res == 0) begin
      errno = $ferror(vfd, result);
      $display("Failed to read back result (%0s)", result);
      str = "failed";
    end
    if (str !== "test-") begin
      $display("$fstrobe was not skipped, expected 'test-', got '%0s'", str);
      pass = 1'b0;
    end
    $fclose(vfd);

    if (pass) $display("PASSED");
  end
endmodule
