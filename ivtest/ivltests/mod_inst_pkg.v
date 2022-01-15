package fooPkg;
    localparam FOO = 5;
endpackage

package barPkg;
    function int get_size (
        input int x
    );
        return x + 3;
    endfunction
endpackage

package bazPkg;
    typedef int baz;
endpackage

/*
IEEE 1800-2012 A.1.2 says:

module_nonansi_header ::=
  { attribute_instance } module_keyword [ lifetime ] module_identifier
    { package_import_declaration } [ parameter_port_list ] list_of_ports ;
module_ansi_header ::=
  { attribute_instance } module_keyword [ lifetime ] module_identifier
    { package_import_declaration } [ parameter_port_list ] [ list_of_port_declarations ] ;

This allows for the importing of packages during module definition which can be used in the
  parameter and port lists.
*/

module foo
// Testing comman separated imports
import
    fooPkg::*,
    barPkg::*;
// Testing multiple import statements
import bazPkg::*;
#(
    parameter FOO_PARAM = FOO
)
(
    input [get_size(7)-1:0] inport
);

    baz value = 11;

    initial begin
        if ($bits(inport) != 10) begin
            $display("FAILED -- function import in module declaration failed (%d)", $bits(inport));
            $finish;
        end

        if (value != 11) begin
            $display("FAILED -- Something is wrong with typedef import (%d)", value);
            $finish;
        end

        if (FOO_PARAM != 5) begin
            $display("FAILED -- Something is wrong with paramater imports (%d)", FOO_PARAM);
            $finish;
        end

        $display("PASSED");
        $finish;
    end

endmodule
