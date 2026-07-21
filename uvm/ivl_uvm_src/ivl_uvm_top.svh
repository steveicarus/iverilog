module ivl_uvm_top;
  import ivl_uvm_pkg::*;

  /*
  `UVM_TESTNAME uvm_test_top;

  initial begin : m_top
    $timeformat (-9, 3, " ns", 3);
    `g2u_printf (( "Using UVM_TESTNAME: %s", `GO2UVM_DISP_ARG (`UVM_TESTNAME) ))
    uvm_test_top = new ("uvm_test_top", null);

  end : m_top
  */

  final begin
    report_summarize();
  end
endmodule : ivl_uvm_top

