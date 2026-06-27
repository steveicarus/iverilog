// Check that nature name fields can shadow visible type identifiers.

typedef int ACCESS_NAME;
typedef int IDT_NAME;
typedef int DDT_NAME;

nature type_id_nature;
  units = "V";
  access = ACCESS_NAME;
  idt_nature = IDT_NAME;
  ddt_nature = DDT_NAME;
endnature

module test;
  initial begin
    $display("PASSED");
  end
endmodule
