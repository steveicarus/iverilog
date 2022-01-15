module top;
  reg pass, pass_f1, pass_f2, pass_f3, pass_f4, pass_f5;
  reg [8*30:1] res;

  initial begin
    pass = 1'b1;

    // Verify that the initial scope is correct.
    $swrite(res, "%m");
    if (res != "top") begin
      $display("Failed initial, got \"%0s\"", res);
      pass = 1'b0;
    end

    // Test %m in a named begin.
    begin : my_begin
      $swrite(res, "%m");
      if (res != "top.my_begin") begin
        $display("Failed named begin (1st), got \"%0s\"", res);
        pass = 1'b0;
      end

      begin : my_begin_begin
        // Test %m in a nested named begin.
        $swrite(res, "%m");
        if (res != "top.my_begin.my_begin_begin") begin
          $display("Failed nested named begin, got \"%0s\"", res);
          pass = 1'b0;
        end
      end

      $swrite(res, "%m");
      if (res != "top.my_begin") begin
        $display("Failed named begin (2nd), got \"%0s\"", res);
        pass = 1'b0;
      end

      // Test a named fork inside a named begin.
      pass_f1 = 1'b1;
      pass_f2 = 1'b1;
      fork : my_begin_fork
        begin
          $swrite(res, "%m");
          if (res != "top.my_begin.my_begin_fork") begin
            $display("Failed after named begin/fork (1), got \"%0s\"", res);
            pass_f1 = 1'b0;
          end
        end
        begin
          $swrite(res, "%m");
          if (res != "top.my_begin.my_begin_fork") begin
            $display("Failed after named begin/fork (2), got \"%0s\"", res);
            pass_f2 = 1'b0;
          end
        end
      join

      pass = pass & pass_f1 & pass_f2;

      $swrite(res, "%m");
      if (res != "top.my_begin") begin
        $display("Failed named begin (3rd), got \"%0s\"", res);
        pass = 1'b0;
      end
    end

    // Verify that the scope is back to normal.
    $swrite(res, "%m");
    if (res != "top") begin
      $display("Failed after named begin, got \"%0s\"", res);
      pass = 1'b0;
    end

    // Test %m in a named fork.
    pass_f1 = 1'b1;
    pass_f2 = 1'b1;
    pass_f3 = 1'b1;
    pass_f4 = 1'b1;
    pass_f5 = 1'b1;
    fork : my_fork
      begin
        $swrite(res, "%m");
        if (res != "top.my_fork") begin
          $display("Failed after named fork (1), got \"%0s\"", res);
          pass_f1 = 1'b0;
        end
      end
      // Test a %m in a nested named begin.
      begin : my_fork_begin
        $swrite(res, "%m");
        if (res != "top.my_fork.my_fork_begin") begin
          $display("Failed after named fork/begin, got \"%0s\"", res);
          pass_f4 = 1'b0;
        end
      end
      begin
        $swrite(res, "%m");
        if (res != "top.my_fork") begin
          $display("Failed after named fork (2), got \"%0s\"", res);
          pass_f2 = 1'b0;
        end
      end
      fork : my_fork_fork
        begin
          $swrite(res, "%m");
          if (res != "top.my_fork.my_fork_fork") begin
            $display("Failed after named fork/fork, got \"%0s\"", res);
            pass_f2 = 1'b0;
          end
        end
      join
      begin
        $swrite(res, "%m");
        if (res != "top.my_fork") begin
          $display("Failed after named fork (3), got \"%0s\"", res);
          pass_f3 = 1'b0;
        end
      end
    join

    pass = pass & pass_f1 & pass_f2 & pass_f3;

    // Verify that the scope is back to normal.
    $swrite(res, "%m");
    if (res != "top") begin
      $display("Failed final, got \"%0s\"", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
