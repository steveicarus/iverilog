// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

                  OVL V2.0 VHDL Implementation User Guide
                  --------------------------------------------
  
Introduction
------------
                 
This is the first release of the VHDL implementation of the OVL and it contains the
top 10 most frequently used checkers from the library. The checkers are the new
ovl_<checker> versions of the components that include the enable and fire ports.

The VHDL components are compatible with the Verilog versions of the components.
The only difference between them is the that VHDL fully implements the fire
outputs and is synthesizable. The VHDL components also have an additional
generic called controls, that allows global configuration of the library.

The VHDL implementation's components support both std_logic/std_logic_vector and
std_ulogic/std_ulogic_vector port types.


Directory Structure
-------------------

The std_ovl directory contains the VHDL entity declarations (ovl_<checker>.vhd),
types/constants package (std_ovl.vhd), procedures package (std_ovl_procs.vhd),
ulogic wrapper components package (std_ovl_u.vhd) and internal gating 
components (std_ovl_clock_gating.vhd and std_ovl_reset_gating.vhd). The 
std_ovl/vhdl93 directory contains the architecture bodies for the entities 
(ovl_<checker>_rtl.vhd). The std_ovl/vhdl93/legacy directory contains the 
std_ovl.vhd file. This file was included in the OVL V1.8 release and allowed 
VHDL designs to instantiate the Verilog assert_<checker> components 
implementations. It is now deprecated and will be removed in the next release.


Tools Supported
---------------

This release has been tested on the following simulators:

   Mentor Graphics ModelSim-VHDL
   Cadence NC-VHDL
   
This release has been tested on the following synthesis tools:

   Synopsys DesignCompiler
   Synplicity SynplifyPro
   

Compilation Library - accellera_ovl_vhdl
----------------------------------------

Accellera mandates that the VHDL OVL implementation is only analyzed into the
logical library name, accellera_ovl_vhdl. We have standardized this name for
portability reasons. It is also expected that, in the future EDA vendors will provide
optimized versions of the library for their tools that uses this library name.
The library only needs to be compiled once because it can be configured via the
use of the ovl_ctrl_record (see below). The library must be compiled using your
EDA tool's VHDL-93 option.


Usage
-----

The VHDL implementation of the OVL has all the configuration features of the 
Verilog implementation that are provided by the macros. For example it is 
possible to globally enable or disable X-checking on all checker instances. 
This is accomplished via the use of the ovl_ctrl_record (defined in std_ovl.vhd)
value assigned to the controls generic on every checker instance. It is highly 
recommended that a ovl_ctrl_record constant is defined in design specific work 
package to be used on all checker instances. This allows the configuration of 
the components to be controlled in one place. The following is an example of how 
to use the ovl_ctrl_record constant:

library accellera_ovl_vhdl;
use accellera_ovl_vhdl.std_ovl.all;

package proj_pkg is

  -- OVL configuration
  constant ovl_proj_controls : ovl_ctrl_record := (
    xcheck_ctrl              => OVL_ON,                                     
    implicit_xcheck_ctrl     => OVL_ON,                                     
    init_msg_ctrl            => OVL_ON,                                     
    init_count_ctrl          => OVL_ON, 
    assert_ctrl              => OVL_ON,                                     
    cover_ctrl               => OVL_OFF,                                     
    global_reset_ctrl        => OVL_OFF,
    finish_ctrl              => OVL_ON,                                     
    
    coverage_level_default   => OVL_COVER_DEFAULT,
    clock_edge_default       => OVL_CLOCK_EDGE_DEFAULT,
    reset_polarity_default   => OVL_RESET_POLARITY_DEFAULT,
    gating_type_default      => OVL_GATING_TYPE_DEFAULT,

    max_report_error         => 10, 
    max_report_cover_point   => 15,

    runtime_after_fatal      => "350 ns    "
  );
  
end package proj_pkg;


library accellera_ovl_vhdl;
use accellera_ovl_vhdl.std_ovl.all;

architecture rtl of design is
begin

  <rtl code>
  
  ovl_gen : if (ovl_proj_controls.assert_ctrl = OVL_ON) generate
    
    <user ovl signal conditioning code>
    
    ovl_u1 : ovl_next
      generic map (
         msg                 => "Check 1",
         num_cks             => 1,
         check_overlapping   => OVL_CHK_OVERLAP_OFF,
         check_missing_start => OVL_OFF,
         coverage_level      => OVL_COVER_CORNER,
         controls            => ovl_proj_controls
      )
      port map (
         clock               => clk,
         reset               => reset_n,
         start_event ,       => start_event_1
         test_expr           => test_1,
         enable              => enable_1,
         fire                => fire_1
      );

    ovl_u2 : ovl_next
      generic map (
         msg                 => "Check 2",
         num_cks             => 2,
         check_overlapping   => OVL_CHK_OVERLAP_ON,
         check_missing_start => OVL_ON,
         coverage_level      => OVL_COVER_ALL,
         severity_level      => OVL_FATAL,
         controls            => ovl_proj_controls
      )
      port map (
         clock               => clk,
         reset               => reset_n,
         start_event         => start_event_2,
         test_expr           => test_2,
         enable              => enable_2,
         fire                => fire_2
      );  
  end generate ovl_gen; 

end architecture rtl;


The ovl_<checker> component versions use std_logic/std_logic_vector port types 
and the ovl_<checker>_u versions of the components use 
std_ulogic/std_ulogic_vector port types. The std_ovl_u.vhd file contains the
std_ovl_u package declaration that declares the std_<checker>_u components and
the entity declarations and architecture bodies for them. To use these components
the following is required in the instantiating code:

library accellera_ovl_vhdl;
use accellera_ovl_vhdl.std_ovl.all;
use accellera_ovl_vhdl.std_ovl_u.all; -- optional - not needed if using direct instantiation


The following shows how to print the total number of OVL checkers initialized 
in a simulation. The init_msg_ctrl and init_count_ctrl items must both be set 
to OVL_ON:

library accellera_ovl_vhdl;
use accellera_ovl_vhdl.std_ovl.all;
use accellera_ovl_vhdl.std_ovl_procs.all;
use work.proj_pkg.all;

entity tb is
end entity tb;

architecture tb of tb is
...
begin
...

  test : process
  begin
     wait for 0 ns;
     ovl_print_init_count_proc(ovl_proj_controls);
     ...  
  end process test;

end architecture tb;


Synthesis
---------  

The VHDL implementation is synthesizable a apart from the path_name attribute 
in the architectures and the std_ovl_procs.vhd file. To resolve this the
architecture bodies must be modified to set the path sting constant to "" and
the std_ovl_procs_syn.vhd file in the std_ovl directory must be used. The path
strings can be modified using the following c-shell script:

setenv OVL <ovl install path>/std_ovl
setenv syn_dir `pwd`
cd ${OVL}/vhdl93
foreach  checker (ovl_*_rtl.vhd)
  sed s/rtl\'path_name/\"\"/ $checker > .${syn_dir}/$checker
end
cd $syn_dir
 

