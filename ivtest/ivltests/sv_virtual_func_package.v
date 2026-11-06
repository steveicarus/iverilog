// Check virtual functions of classes declared in a package.

package pkg;
  class Base;
    virtual function string who();
      return "Base";
    endfunction
    function string describe();
      return {"I am ", who()};
    endfunction
  endclass

  class Derived extends Base;
    virtual function string who();
      return "Derived";
    endfunction
  endclass
endpackage

module test;
  pkg::Base b;
  pkg::Derived d;

  initial begin
    d = new;
    b = d;
    if (b.who() == "Derived" && b.describe() == "I am Derived")
      $display("PASSED");
    else
      $display("FAILED: %0s", b.describe());
  end
endmodule
