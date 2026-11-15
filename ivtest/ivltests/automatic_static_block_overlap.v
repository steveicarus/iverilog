module automatic_static_block_overlap;
  integer total = 0;

  task run(input integer value);
    begin : local_vars
      automatic integer saved = value;
      automatic real fraction = value + 0.5;
      automatic string text = "local";
      #5;
      if (fraction != saved + 0.5 || text != "local")
        $fatal(1, "overlapping calls shared automatic storage");
      total = total + saved;
    end
  endtask

  initial begin
    fork
      run(1);
      begin
        #1;
        run(2);
      end
    join
    if (total != 3) $fatal(1, "total=%0d", total);
    run(4);
    if (total != 7) $fatal(1, "reused context: total=%0d", total);
    $display("PASSED");
  end
endmodule
