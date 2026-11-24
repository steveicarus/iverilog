// Check that a member of a virtual interface cannot be passed to an output
// argument of a task.
interface bus_if;
  logic [7:0] data;
endinterface

module test;
  bus_if b0();
  virtual bus_if v;

  task set(output logic [7:0] o);
    o = 8'h12;
  endtask

  initial begin
    v = b0;
    set(v.data);
    $display("FAILED");
  end
endmodule
