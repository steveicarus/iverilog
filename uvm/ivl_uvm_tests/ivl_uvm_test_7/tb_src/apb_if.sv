
//interface apb_if (input bit pclk);
interface apb_if ();
   bit pclk;
   bit presetn;
   logic [7:0] paddr;
   logic        psel;
   logic        penable;
   logic        pwrite;
   logic [7:0] prdata;
   logic [7:0] pwdata;
   logic        pready;
   logic        pslverr;

endinterface : apb_if

