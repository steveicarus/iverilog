// Check that an undefined named function return type fails during elaboration.

module test;

  function missing_type f;
    f = 0;
  endfunction

endmodule
