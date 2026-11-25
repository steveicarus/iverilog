// Check that a virtual interface can be passed to ref and const ref arguments.
interface bus_if;
  logic [7:0] data;
endinterface

module test;
  bus_if b0(), b1();
  virtual bus_if v;
  bit failed = 0;

  task automatic set_handle(ref virtual bus_if h);
    h = b1;
  endtask

  function automatic logic [7:0] get_data(const ref virtual bus_if h);
    return h.data;
  endfunction

  initial begin
    b0.data = 8'h11;
    b1.data = 8'h22;
    v = b0;
    if (get_data(v) !== 8'h11) failed = 1;
    set_handle(v);
    if (v != b1) failed = 1;
    if (get_data(v) !== 8'h22) failed = 1;
    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end
endmodule
