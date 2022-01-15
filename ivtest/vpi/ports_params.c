/*
 * Copyright (c) 2012  Andrew Stevens wackston@googlemail.com
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


#include <assert.h>
#include "vpi_user.h"


static void checkParams(void)
{
    vpiHandle mod_i = vpi_iterate(vpiModule, NULL) ;
    assert( mod_i != NULL );
    vpiHandle module = vpi_scan(mod_i);
    assert( module != NULL );
    vpi_free_object(mod_i);

    vpiHandle param_i = vpi_iterate(vpiParameter, module) ;
    assert( param_i != NULL );
    vpiHandle parameter;
    while ( (parameter=vpi_scan(param_i))!=NULL)
    {
        char *name =vpi_get_str(vpiName, parameter) ;
        int type = vpi_get(vpiConstType, parameter) ;
        printf( "PARAM NAME=%s type=%d ",  name, type );
        s_vpi_value val ;

        switch(type)
        {
        case vpiDecConst:
        case vpiBinaryConst:
        case vpiOctConst:
        case vpiHexConst:       val.format = vpiIntVal ;
                                vpi_get_value(parameter, &val) ;
                                printf( "value=(INT)%d ", val.value.integer );
                                break ;

        case vpiRealConst:      val.format = vpiRealVal ;
                                vpi_get_value(parameter, &val) ;
                                printf( "value=(REAL)%g ", val.value.real );
                                break ;

        case vpiStringConst:    val.format = vpiStringVal ;
                                vpi_get_value(parameter, &val) ;
                                printf( "value=(STR)\"%s\" ", val.value.str );
                                break;
        default:                printf( "value=<UNKNOWN> " );
                                break ;
        }


        int local =vpi_get(vpiLocalParam, parameter)!=0 ;
        printf( "local=%s\n", local ? "yes" : "no" );
    }
}



static void checkPorts(void)
{
    vpiHandle mod_i = vpi_iterate(vpiModule, NULL) ;
    assert( mod_i != 0 );
    vpiHandle module = vpi_scan(mod_i);
    assert( module != 0 );
    vpi_free_object(mod_i) ;


    vpiHandle port_i = vpi_iterate(vpiPort, module) ;
    vpiHandle port ;
    while ( (port=vpi_scan(port_i))!=NULL)
    {
        char *portName = vpi_get_str(vpiName, port) ;
        int portIndex = vpi_get(vpiPortIndex, port) ;
        PLI_INT32 dir = vpi_get(vpiDirection, port) ;
        PLI_INT32 size = vpi_get(vpiSize, port) ;

        printf( "PORT name=%s index=%d dir=%d size=%d\n", portName, portIndex, dir, size );
    }
}



static PLI_INT32 checkPortsParams(struct t_cb_data*cb)
{
    (void)cb;  /* Parameter is not used. */

    checkParams();
    checkPorts();
    return 0;
}


static void setCallback(void)
{
    s_cb_data   cb ;   /* setup a callback for start of simulation */
    cb.reason    = cbStartOfSimulation ;
    cb.cb_rtn    = checkPortsParams;
    cb.user_data = "checkPortsParams" ;
    cb.obj       = NULL ;
    cb.time      = NULL ;
    cb.value     = NULL ;

    vpi_register_cb(&cb) ;
}


void (*vlog_startup_routines[]) (void) = { setCallback, 0};
