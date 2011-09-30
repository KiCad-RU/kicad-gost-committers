/**
 * @file eeschema/cross-probing.cpp
 */

#include "fctsys.h"
#include "appl_wxstruct.h"
#include "macros.h"
#include "eda_dde.h"
#include "wxEeschemaStruct.h"

#include "general.h"
#include "eeschema_id.h"
#include "protos.h"
#include "lib_draw_item.h"
#include "lib_pin.h"
#include "sch_component.h"


/**
 * Read a remote command sent by Pcbnew (via a socket connection) ,
 * so when user selects a module or pin in Pcbnew,
 * Deschema shows that same component or pin.
 * The cursor is put on the item
 *  port KICAD_SCH_PORT_SERVICE_NUMBER (currently 4243)
 * @param cmdline = received command from Pcbnew
 * commands are:
 * $PART: "reference"   put cursor on component
 * $PART: "reference" $REF: "ref"  put cursor on reference component
 * $PART: "reference" $VAL: "value" put cursor on value component
 * $PART: "reference" $PAD: "pin name"  put cursor on the component pin
 */
void RemoteCommand( const char* cmdline )
{
    char     line[1024];
    char*    idcmd;
    char*    text;
    wxString part_ref, msg;
    SCH_EDIT_FRAME* frame;

    frame = (SCH_EDIT_FRAME*)wxGetApp().GetTopWindow();

    strncpy( line, cmdline, sizeof(line) - 1 );

    idcmd = strtok( line, " \n\r" );
    text  = strtok( NULL, "\"\n\r" );

    if( (idcmd == NULL) || (text == NULL) )
        return;

    if( strcmp( idcmd, "$PART:" ) != 0 )
        return;

    part_ref = FROM_UTF8( text );

    /* look for a complement */
    idcmd = strtok( NULL, " \n\r" );

    if( idcmd == NULL )    // component only
    {
        frame->FindComponentAndItem( part_ref, true, 0, wxEmptyString, false );
        return;
    }

    text = strtok( NULL, "\"\n\r" );

    if( text == NULL )
        return;

    msg = FROM_UTF8( text );

    if( strcmp( idcmd, "$REF:" ) == 0 )
    {
        frame->FindComponentAndItem( part_ref, true, 2, msg, false );
    }
    else if( strcmp( idcmd, "$VAL:" ) == 0 )
    {
        frame->FindComponentAndItem( part_ref, true, 3, msg, false );
    }
    else if( strcmp( idcmd, "$PAD:" ) == 0 )
    {
        frame->FindComponentAndItem( part_ref, true, 1, msg, false );
    }
    else
        frame->FindComponentAndItem( part_ref, true, 0, wxEmptyString, false );
}


/** Send a remote command to Eeschema via a socket,
 * @param objectToSync = item to be located on board (footprint, pad or text)
 * @param LibItem = component in lib if objectToSync is a sub item of a component
 * Commands are
 * $PART: reference   put cursor on footprint anchor
 * $PIN: number $PART: reference put cursor on the footprint pad
 */
void SCH_EDIT_FRAME::SendMessageToPCBNEW( EDA_ITEM* objectToSync, SCH_COMPONENT*  LibItem )
{
    if( objectToSync == NULL )
        return;

    LIB_PIN* Pin = NULL;
    char     Line[1024];

    /* Cross probing to Pcbnew if a pin or a component is found */
    switch( objectToSync->Type() )
    {
    case SCH_FIELD_T:
    case LIB_FIELD_T:
    {
        if( LibItem == NULL )
            break;

        sprintf( Line, "$PART: %s", TO_UTF8( LibItem->GetField( REFERENCE )->m_Text ) );
        SendCommand( MSG_TO_PCB, Line );
    }
    break;

    case SCH_COMPONENT_T:
        LibItem = (SCH_COMPONENT*) objectToSync;
        sprintf( Line, "$PART: %s", TO_UTF8( LibItem->GetField( REFERENCE )->m_Text ) );
        SendCommand( MSG_TO_PCB, Line );
        break;

    case LIB_PIN_T:
        if( LibItem == NULL )
            break;

        Pin = (LIB_PIN*) objectToSync;

        if( Pin->GetNumber() )
        {
            wxString pinnum;
            Pin->ReturnPinStringNum( pinnum );
            sprintf( Line, "$PIN: %s $PART: %s", TO_UTF8( pinnum ),
                     TO_UTF8( LibItem->GetField( REFERENCE )->m_Text ) );
        }
        else
        {
            sprintf( Line, "$PART: %s", TO_UTF8( LibItem->GetField( REFERENCE )->m_Text ) );
        }

        SendCommand( MSG_TO_PCB, Line );
        break;

    default:
        break;
    }
}
