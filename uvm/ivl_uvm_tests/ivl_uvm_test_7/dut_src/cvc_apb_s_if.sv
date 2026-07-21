/*
//APB (Advanced peripheral Bus) Interface 
//
*/

`ifndef APB_IF_SV
`define APB_IF_SV

interface apb_if(input bit pclk);
  logic presetn;
  logic [31:0] paddr;
  logic        psel;
  logic        penable;
  logic        pwrite;
  logic [31:0] prdata;
  logic [31:0] pwdata;

   //Slave Clocking block
  clocking slave_cb @(posedge pclk);
      input  paddr, psel, penable, pwrite, pwdata;
      output prdata;
  endclocking: slave_cb


   //Monitor Clocking block - For sampling by monitor components
   clocking monitor_cb @(posedge pclk);
      input paddr, psel, penable, pwrite, prdata, pwdata;
   endclocking: monitor_cb


endinterface: apb_if

`endif // APB_IF_SV
