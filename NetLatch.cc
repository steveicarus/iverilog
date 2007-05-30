// NetLatch.cc
// Author: Alan M. Feldstein
// Class NetLatch member-function definitions

#include "NetLatch.h" // NetLatch class definition

// constructor
NetLatch::NetLatch( NetScope *scope, perm_string name, unsigned width )
  // explicitly call base-class constructor
  : NetNode( scope, name, 2U * width + 1U )
{
} // end NetLatch constructor

Link &NetLatch::pin_Data( unsigned w )
{
  unsigned pn = 1 + 2 * w;
  assert( pn < pin_count() );
  return pin( pn );
} // end function pin_Data

Link &NetLatch::pin_Q( unsigned w )
{
  unsigned pn = 2 + 2 * w;
  assert( pn < pin_count() );
  return pin( pn );
} // end function pin_Q

Link &NetLatch::pin_Gate()
{
  return pin( 0 );
} // end function pin_Q
