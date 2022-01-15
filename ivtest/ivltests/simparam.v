`timescale 1ns/1ps

module top;
  lower dut();
endmodule

module lower;
  parameter bad_name = "this_is_a_bad_name";
  reg [15:0] def = "OK";
  reg pass = 1'b1;
  reg [1023:0] result;
  real rl_res;

  initial begin

    #1;

    /* Display the version and other information. */
    $display("Testing with Icarus Verilog version: %g, subversion: %g",
             $simparam("simulatorVersion"), $simparam("simulatorSubversion"));
    $display("Using a CPU word size of %g bits.", $simparam("CPUWordSize"));
    $display("Running in directory: %0s\n", $simparam$str("cwd"));

    /*
     * Check the time units and precision.
     *
     * Since this is double math check that the result is within a
     * factor of 1e-10 of the correct value.
     */
    rl_res = $simparam("timeUnit") - 1e-9;
    rl_res = (rl_res < 0) ? -rl_res : rl_res;
    if (rl_res > 1e-9*1e-10) begin
      $display("$simparam(\"timeUnit\") failed, got %g.", rl_res);
      pass = 1'b0;
    end

    rl_res = $simparam("timePrecision") - 1e-12;
    rl_res = (rl_res < 0) ? -rl_res : rl_res;
    if (rl_res >= 1e-12*1e-10) begin
      $display("$simparam(\"timePrecision\") failed, got %g.", rl_res);
      pass = 1'b0;
    end

    /* Check the string routines, see below for why this is a task. */
    check_string;

    /* Check that a bad parameter name with a default works. */
    if ($simparam(bad_name, 1.0) != 1.0) begin
      $display("$simparam with a bad name and a default value failed.");
      pass = 1'b0;
    end

    result = $simparam$str(bad_name, def);
    if (result[15:0] != "OK") begin
      $display("$simparam$str with a bad name and a default value failed.");
      pass = 1'b0;
    end

    /* These should also print an error message. */
    if ($simparam(bad_name) != 0.0) begin
      $display("$simparam with a bad name failed.");
      pass = 1'b0;
    end

    result = $simparam$str(bad_name);
    if (result[55:0] != "<error>") begin
      $display("$simparam$str with a bad name failed.");
      pass = 1'b0;
    end


    /* All these are currently unimplemented and just return 0.0 or N/A. */
    if ($simparam("gdev") != 0.0) begin
      $display("$simparam(\"gdev\") failed.");
      pass = 1'b0;
    end

    if ($simparam("gmin") != 0.0) begin
      $display("$simparam(\"gmin\") failed.");
      pass = 1'b0;
    end

    if ($simparam("imax") != 0.0) begin
      $display("$simparam(\"imax\") failed.");
      pass = 1'b0;
    end

    if ($simparam("imelt") != 0.0) begin
      $display("$simparam(\"imelt\") failed.");
      pass = 1'b0;
    end

    if ($simparam("iteration") != 0.0) begin
      $display("$simparam(\"iteration\") failed.");
      pass = 1'b0;
    end

    if ($simparam("scale") != 0.0) begin
      $display("$simparam(\"scale\") failed.");
      pass = 1'b0;
    end

    if ($simparam("shrink") != 0.0) begin
      $display("$simparam(\"shrink\") failed.");
      pass = 1'b0;
    end

    if ($simparam("sourceScaleFactor") != 0.0) begin
      $display("$simparam(\"sourceScaleFactor\") failed.");
      pass = 1'b0;
    end

    if ($simparam("tnom") != 0.0) begin
      $display("$simparam(\"tnom\") failed.");
      pass = 1'b0;
    end

    result = $simparam$str("analysis_name");
    if (result[23:0] != "N/A") begin
      $display("$simparam$str(\"analysis_name\") failed.");
      pass = 1'b0;
    end

    result = $simparam$str("analysis_type");
    if (result[23:0] != "N/A") begin
      $display("$simparam$str(\"analysis_type\") failed.");
      pass = 1'b0;
    end

    if (pass) $display("\nPASSED");

  end

  /* We need this to make instance and path different. */
  task check_string;
    begin
      result = $simparam$str("module");
      if (result[39:0] != "lower") begin
        $display("$simparam$str(\"module\") failed, got %0s.", result);
        pass = 1'b0;
      end

      result = $simparam$str("instance");
      if (result[55:0] != "top.dut") begin
        $display("$simparam$str(\"instance\") failed, got %0s.", result);
        pass = 1'b0;
      end

      result = $simparam$str("path");
      if (result[159:0] != "top.dut.check_string") begin
        $display("$simparam$str(\"instance\") failed, got %0s.", result);
        pass = 1'b0;
      end
    end
  endtask

endmodule
