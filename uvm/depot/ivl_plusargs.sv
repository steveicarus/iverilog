module ivl_plusargs;
  string var1, var2, var3, var4, var5, var6, var7, var8, var9;
  // Written by Anirudh.
  initial begin
    $display ("Initializing.....");
    if ($value$plusargs ("UVM_DUMP_CMDLINE_ARGS=%s", var1))
      $display ("STRING with UVM_DUMP_CMDLINE_ARGS has a value %s", var1);
    
    if ($value$plusargs ("UVM_TESTNAME=%s", var2))
      $display ("STRING with UVM_TESTNAME has a value %s", var2);
   
    if ($value$plusargs ("UVM_VERBOSITY=%s", var3))
      $display ("STRING with UVM_VERBOSITY has a value %s", var3);

    if ($value$plusargs ("UVM_TIMEOUT=%s", var4))
      $display ("STRING with UVM_TIMEOUT has a value %s", var4);

    if ($value$plusargs ("UVM_MAX_QUIT_COUNT=%s", var5))
      $display ("STRING with UVM_MAX_QUIT_COUNT has a value %s", var5);

    if ($value$plusargs ("UVM_PHASE_TRACE=%s", var6))
      $display ("STRING with UVM_PHASE_TRACE has a value %s", var6);

    if ($value$plusargs ("UVM_OBJECTION_TRACE=%s", var7))
      $display ("STRING with UVM_OBJECTION_TRACE has a value %s", var7);

    if ($value$plusargs ("UVM_RESOURCE_DB_TRACE=%s", var8))
      $display ("STRING with UVM_RESOURCE_DB_TRACE has a value %s", var8);

    if ($value$plusargs ("UVM_CONFIG_DB_TRACE=%s", var9))
      $display ("STRING with UVM_CONFIG_DB_TRACE has a value %s", var9);
  end
endmodule : ivl_plusargs
