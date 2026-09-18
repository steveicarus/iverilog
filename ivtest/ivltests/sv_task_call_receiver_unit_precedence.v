// Check that a compilation-unit object takes precedence over an enclosing
// instance when it is the receiver of a task call.

class C;

  integer value = 0;

  task set_value;
    value = 11;
  endtask

endclass

C object = new;

module holder;
endmodule

module child;

  initial begin
    object.set_value();

    if (object.value !== 11) begin
      $display("FAILED: value=%0d", object.value);
    end else begin
      $display("PASSED");
    end
  end

endmodule

module test;

  holder object();
  child child_instance();

endmodule
