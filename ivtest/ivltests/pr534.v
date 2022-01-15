///////////////////////////////////////////////////////////////////////////
//
// To test:
//        (a) The use & representation of time variables
//        (b) The display of time variables
//
// Compile and run the program
//        iverilog tt_clean.v
//        vvp a.out
//
// VISUALLY INSPECT the displays.  (There ain't no way to automate this)
//
///////////////////////////////////////////////////////////////////////////
`timescale 1 ns / 10 ps

`define	PCI_CLK_PERIOD		15.0		// 66 Mhz

module top;
  reg PCI_Clk;
  reg fail;

  initial PCI_Clk <= 0;
  always #(`PCI_CLK_PERIOD/2) PCI_Clk <= ~PCI_Clk;

  initial begin
    fail = 0;
    $display("\n\t\t==> CHECK THIS DISPLAY ==>\n");
    $display("pci_clk_period:\t\t\t %0d",`PCI_CLK_PERIOD);
    $display("pci_clk_period:\t\t\t %0t",`PCI_CLK_PERIOD);
    if($time !== 0) fail = 1;
    if (fail == 1)
      $display("$time=%0d (0)", $time);
    delay_pci(3);
    if($simtime !== 4500) fail = 1;
    if($time !== 45) fail = 1;
    if (fail == 1)
      $display("$time=%0d (45)", $time);
    #15;
    if($simtime !== 6000) fail = 1;
    if($time !== 60) fail = 1;
    #(`PCI_CLK_PERIOD);
    if($simtime !== 7500) fail = 1;
    if($time !== 75) fail = 1;
    #(`PCI_CLK_PERIOD *2);
    if($simtime !== 10500) fail = 1;
    if($time !== 105) fail = 1;

    $timeformat(-9,2,"ns",20);
    $display("after setting timeformat:");
    $display("pci_clk_period:\t\t\t %0d",`PCI_CLK_PERIOD);
    $display("pci_clk_period:\t\t\t %0t",`PCI_CLK_PERIOD);
    delay_pci(3);
    if($simtime !== 15000) fail = 1;
    if($time !== 150) fail = 1;
    #15;
    if($simtime !== 16500) fail = 1;
    if($time !== 165) fail = 1;
    #(`PCI_CLK_PERIOD);
    if($simtime !== 18000) fail = 1;
    if($time !== 180) fail = 1;
    #(`PCI_CLK_PERIOD *2);
    if($simtime !== 21000) fail = 1;
    if($time !== 210) fail = 1;

    $display("\t\t**********************************************");
    if(fail) $display("\t\t****** time representation test BAD    *******");
    else     $display("\t\t****** time representation test OK     *******");
    $display("\t\t**********************************************\n");
    $finish(0);
  end

  task delay_pci;
    input delta;
    integer delta;
    integer ii;
    begin
      #(`PCI_CLK_PERIOD * delta);
    end
  endtask

endmodule
