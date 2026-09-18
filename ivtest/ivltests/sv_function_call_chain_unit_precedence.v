// Check that compilation-unit function lookup is used at the start of a
// chained call.

class unit_class;

  function integer value;
    value = 1;
  endfunction

endclass

module child;

  initial begin
    if (make_object().value() == 1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule

function unit_class make_object;
  make_object = new;
endfunction

module test;

  class instance_class;

    function integer value;
      value = 2;
    endfunction

  endclass

  child i_child();

  function instance_class make_object;
    make_object = new;
  endfunction

endmodule
