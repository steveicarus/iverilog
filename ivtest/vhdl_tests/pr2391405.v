/*----------------------------------------------------------------------
 *
 * module description here
 *
 *---------------------------------------------------------------------*/


module foo

  (/*AUTOARG*/
  // Outputs
  result_0, result_1,
  // Inputs
  clk, reset, enable_a
  );

  //----------------------------------------
  input                 clk;

  input                 reset;

  input                 enable_a;

  output                result_0;
  output                result_1;
  //----------------------------------------

  /*----------------------------------------------------------------*/

  /*-AUTOUNUSED-*/

  /*AUTOINPUT*/

  /*AUTOOUTPUT*/

  /*-AUTOREGINPUT-*/

  /*AUTOREG*/

  /*AUTOWIRE*/

  /*------------------------------------------------------------------
   *
   * local definitions and connections.
   *
   * */

  wire                  enable_0, enable_1;

  assign                enable_0 = 1'd1;
  assign                enable_1 = enable_a;

  /*------------------------------------------------------------------
   *
   *
   *
   * */

  /* bar AUTO_TEMPLATE (
   .enable (enable_@),
   .result (result_@),
   ) */
  bar bar0
    (/*AUTOINST*/
     // Outputs
     .result                            (result_0),              // Templated
     // Inputs
     .clk                               (clk),
     .enable                            (enable_0),              // Templated
     .reset                             (reset));

  bar bar1
    (/*AUTOINST*/
     // Outputs
     .result                            (result_1),              // Templated
     // Inputs
     .clk                               (clk),
     .enable                            (enable_1),              // Templated
     .reset                             (reset));

  /*----------------------------------------------------------------*/

endmodule // foo


// Local Variables:
// verilog-library-directories:(".")
// verilog-library-extensions:(".v")
// End:
/*----------------------------------------------------------------------
 *
 * module description here
 *
 *---------------------------------------------------------------------*/


module bar

  (/*AUTOARG*/
  // Outputs
  result,
  // Inputs
  clk, enable, reset
  );

  //----------------------------------------
  input                 clk;
  input                 enable;

  input                 reset;

  output                result;
  //----------------------------------------

  /*----------------------------------------------------------------*/

  /*-AUTOUNUSED-*/

  /*AUTOINPUT*/

  /*AUTOOUTPUT*/

  /*-AUTOREGINPUT-*/

  /*AUTOREG*/
  // Beginning of automatic regs (for this module's undeclared outputs)
  reg                   result;
  // End of automatics

  /*AUTOWIRE*/

  /*------------------------------------------------------------------
   *
   * local definitions and connections.
   *
   * */

  /*------------------------------------------------------------------
   *
   *
   *
   * */

  always @ (posedge clk) begin
    if (reset) begin
      result <= 1'd0;
    end else begin
      result <= enable;
    end
  end

  /*----------------------------------------------------------------*/

endmodule // bar


// Local Variables:
// verilog-library-directories:(".")
// verilog-library-extensions:(".v")
// End:
