// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

// Parameters that should not be edited
  
  parameter OVL_COVER_SANITY_ON    = (coverage_level & `OVL_COVER_SANITY);
  parameter OVL_COVER_BASIC_ON     = (coverage_level & `OVL_COVER_BASIC);
  parameter OVL_COVER_CORNER_ON    = (coverage_level & `OVL_COVER_CORNER);
  parameter OVL_COVER_STATISTIC_ON = (coverage_level & `OVL_COVER_STATISTIC);
