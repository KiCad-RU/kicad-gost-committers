/**
 * @file eeschema_config.h
 */

#include <config_params.h>

#define GROUP       wxT( "/eeschema" )
#define GROUPCOMMON wxT( "/common" )
#define GROUPLIB    wxT( "libraries" )

/* saving parameters option : */
#define INSETUP true    /* used when the parameter is saved in general config
                         * if not used, the parameter is saved in the local
                         * config (project config) */
