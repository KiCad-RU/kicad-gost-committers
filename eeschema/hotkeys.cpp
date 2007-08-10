	/***************/
	/* hotkeys.cpp */
	/***************/

#include "fctsys.h"

#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "id.h"

#include "protos.h"

enum hotkey_id_commnand {
	HK_NOT_FOUND = 0,
	HK_HELP,
	HK_ZOOM_IN,
	HK_ZOOM_OUT,
	HK_ZOOM_REDRAW,
	HK_ZOOM_CENTER,
	HK_NEXT_SEARCH,
	HK_DELETE,
	HK_REPEAT_LAST,
	HK_MOVEBLOCK_TO_DRAGBLOCK,
	HK_ROTATE_COMPONENT,
	HK_MIRROR_X_COMPONENT,
	HK_MIRROR_Y_COMPONENT,
	HK_ORIENT_NORMAL_COMPONENT,
	HK_MOVE_COMPONENT,
	HK_ADD_NEW_COMPONENT,
	HK_BEGIN_WIRE
};

/* Class to handle hotkey commnands. hotkeys have a default value
This class allows (for the future..) the real key code changed by user(from a key code list file, TODO)
*/
class Ki_HotkeyInfo
{
public:
	int m_KeyCode;					// Key code (ascii value for ascii keys or wxWidgets code for function key
	wxString m_InfoMsg;				// info message.
	hotkey_id_commnand m_Idcommand;	// internal id for the corresponding command (see hotkey_id_commnand list)

public:
	Ki_HotkeyInfo(const wxChar * infomsg, hotkey_id_commnand idcommand, int keycode);
};

Ki_HotkeyInfo::Ki_HotkeyInfo(const wxChar * infomsg, hotkey_id_commnand idcommand, int keycode)
{
	m_KeyCode = keycode;		// Key code (ascii value for ascii keys or wxWidgets code for function key
	m_InfoMsg = infomsg;		// info message.
	m_Idcommand = idcommand;	// internal id for the corresponding command (see hotkey_id_commnand list)
}

/* local variables */
/* Hotkey list: */
static Ki_HotkeyInfo HkBeginWire(wxT("begin Wire"), HK_BEGIN_WIRE, 'W');
static Ki_HotkeyInfo HkAddComponent(wxT("Add Component"), HK_ADD_NEW_COMPONENT, 'A');
static Ki_HotkeyInfo HkMirrorYComponent(wxT("Mirror Y Component"), HK_MIRROR_Y_COMPONENT, 'Y');
static Ki_HotkeyInfo HkMirrorXComponent(wxT("Mirror X Component"), HK_MIRROR_X_COMPONENT, 'X');
static Ki_HotkeyInfo HkOrientNormalComponent(wxT("Orient Normal Component"), HK_ORIENT_NORMAL_COMPONENT, 'N');
static Ki_HotkeyInfo HkRotateComponent(wxT("Rotate Component"), HK_ROTATE_COMPONENT, 'R');
static Ki_HotkeyInfo HkMoveComponent(wxT("Move Component"), HK_MOVE_COMPONENT, 'M');
static Ki_HotkeyInfo HkMove2Drag(wxT("Switch move block to drag block"), HK_MOVEBLOCK_TO_DRAGBLOCK, '\t');
static Ki_HotkeyInfo HkInsert(wxT("Repeat Last Item"), HK_REPEAT_LAST, WXK_INSERT);
static Ki_HotkeyInfo HkDelete(wxT("Delete Item"), HK_DELETE, WXK_DELETE);
static Ki_HotkeyInfo HkNextSearch(wxT("Next Search"), HK_NEXT_SEARCH, WXK_F5);
static Ki_HotkeyInfo HkZoomCenter(wxT("Zoom Center"), HK_ZOOM_CENTER, WXK_F4);
static Ki_HotkeyInfo HkZoomRedraw(wxT("Zoom Redraw"), HK_ZOOM_REDRAW, WXK_F3);
static Ki_HotkeyInfo HkZoomOut(wxT("Zoom Out"), HK_ZOOM_OUT, WXK_F2);
static Ki_HotkeyInfo HkZoomIn(wxT("Zoom In"), HK_ZOOM_IN, WXK_F1);
static Ki_HotkeyInfo HkHelp(wxT("Help: this message"), HK_HELP, '?');

// List of hotkey descriptors for schematic
static Ki_HotkeyInfo *s_Schematic_Hotkey_List[] = {
	&HkHelp,
	&HkZoomIn, &HkZoomOut, &HkZoomRedraw, &HkZoomCenter,
	&HkNextSearch, &HkDelete, &HkInsert, &HkMove2Drag,
	&HkMoveComponent, &HkAddComponent,
	&HkRotateComponent, &HkMirrorXComponent, &HkMirrorYComponent, & HkOrientNormalComponent,
	&HkBeginWire,
	NULL
};


// Library editor:
static Ki_HotkeyInfo HkInsertPin(wxT("Repeat Pin"), HK_REPEAT_LAST, WXK_INSERT);

// List of hotkey descriptors for libray editor
static Ki_HotkeyInfo *s_LibEdit_Hotkey_List[] =
{
	&HkHelp,
	&HkZoomIn, &HkZoomOut, &HkZoomRedraw, &HkZoomCenter,
	&HkInsertPin,
	NULL
};


/****************************************************/
static wxString ReturnKeyNameFromKeyCode(int keycode)
/****************************************************/
/*
	* return the key name from the key code
	* Only some wxWidgets key values are handled for function key
	* @param key = key code (ascii value, or wxWidgets value for function keys)
	* @return the key name wxString
*/
{
wxString keyname, modifier, fullkeyname;
	
	if ( keycode & GR_KB_CTRL) modifier << wxT("Ctrl ");
	if ( keycode & GR_KB_ALT) modifier << wxT("Alt ");
	if ( keycode & GR_KB_SHIFT) modifier << wxT("Shift ");
	keycode &= ~(GR_KB_CTRL|GR_KB_ALT|GR_KB_SHIFT);

	switch ( keycode )
	{
		default:
			keyname.Printf(wxT("%c"), keycode);
			break;
		
		case WXK_F1:
		case WXK_F2:
		case WXK_F3:
		case WXK_F4:
		case WXK_F5:
		case WXK_F6:
		case WXK_F7:
		case WXK_F8:
		case WXK_F9:
		case WXK_F10:
		case WXK_F11:
		case WXK_F12:
			keyname.Printf(wxT("F%d"), keycode - WXK_F1 + 1);
			break;
			
		case '\t':
			keyname = wxT("Tab");
			break;

		case WXK_DELETE:
			keyname = wxT("Delete");
			break;

		case WXK_INSERT:
			keyname = wxT("Insert");
			break;
	}
	
	fullkeyname = modifier + keyname;
	return keyname;
}

/****************************************************************************/
static void DisplayHotkeyList(WinEDA_DrawFrame * frame, Ki_HotkeyInfo ** List)
/*****************************************************************************/
/*
	* Displays the current hotkey list
	* @param frame = current open frame
	* @param List = pointer to a Ki_HotkeyInfo list of commands
	* @return none
*/
{
wxString keyname;
	
	wxString msg = _("Current hotkey list:\n\n");
	for ( ; * List != NULL; List++ )
	{
		Ki_HotkeyInfo * hk_decr = * List;
		if ( hk_decr->m_InfoMsg.IsEmpty() ) break;
		msg += _("key ");
		keyname = ReturnKeyNameFromKeyCode(hk_decr->m_KeyCode);
		msg += keyname + wxT(": ") + hk_decr->m_InfoMsg + wxT("\n");
	}
	DisplayInfo(frame, msg);
}

/******************************************************************/
static int GetCommandCodeFromHotkey(int key, Ki_HotkeyInfo ** List)
/******************************************************************/
/*
	* Return an id identifier fron a key code for OnHotKey() function
	* @param key = key code (ascii value, or wxWidgets value for function keys
	* @param List = pointer to a Ki_HotkeyInfo list of commands
	* @return the corresponding function identifier from the Ki_HotkeyInfo  List
*/
{
	for ( ; * List != NULL; List++ )
	{
		Ki_HotkeyInfo * hk_decr = * List;
		if ( hk_decr->m_KeyCode == key ) return hk_decr->m_Idcommand;
	}
	
	return HK_NOT_FOUND;
}

/***********************************************************/
void WinEDA_SchematicFrame::OnHotKey(wxDC * DC, int hotkey,
					EDA_BaseStruct * DrawStruct)
/***********************************************************/
/* Hot keys. Some commands are relatives to the item under the mouse cursor
	Commands are case insensitive
	Zoom commands are not managed here
*/
{
bool PopupOn = m_CurrentScreen->m_CurrentItem  &&
			m_CurrentScreen->m_CurrentItem->m_Flags;
bool RefreshToolBar = FALSE;	// We must refresh tool bar when the undo/redo tool state is modified
	
	if ( hotkey == 0 ) return;

wxPoint MousePos = m_CurrentScreen->m_MousePosition; 

	/* Convert lower to upper case (the usual toupper function has problem with non ascii codes like function keys */
	if( (hotkey >= 'a') && (hotkey <= 'z') ) hotkey += 'A' - 'a';
	
	// Search commnd from key :
	switch ( GetCommandCodeFromHotkey(hotkey, s_Schematic_Hotkey_List) )
	{
		default:
		case HK_NOT_FOUND:
			return;
			break;
		
		case HK_HELP:	// Display Current hotkey list
			DisplayHotkeyList(this, s_Schematic_Hotkey_List);
			break;

		case HK_ZOOM_IN:
		case HK_ZOOM_OUT:
		case HK_ZOOM_REDRAW:
		case HK_ZOOM_CENTER:
			break;

		case HK_MOVEBLOCK_TO_DRAGBLOCK:    // Switch to drag mode, when block moving
			HandleBlockEndByPopUp(BLOCK_DRAG, DC);
			break;

		case HK_DELETE:
			if ( PopupOn ) break;
			RefreshToolBar = LocateAndDeleteItem(this, DC);
			m_CurrentScreen->SetModify();
			m_CurrentScreen->m_CurrentItem = NULL;
			TestDanglingEnds(m_CurrentScreen->EEDrawList, DC);
			break;

		case HK_REPEAT_LAST:
			if ( g_ItemToRepeat && (g_ItemToRepeat->m_Flags == 0)  )
			{
				RepeatDrawItem(DC);
			}
			else wxBell();
			break;

		case HK_NEXT_SEARCH :
			if ( g_LastSearchIsMarker ) WinEDA_SchematicFrame::FindMarker(1);
			else FindSchematicItem(wxEmptyString, 2);
			break;

        case HK_ADD_NEW_COMPONENT:	// Add component
			if ( DrawStruct && DrawStruct->m_Flags ) break;
            // switch to m_ID_current_state = ID_COMPONENT_BUTT;
            if ( m_ID_current_state != ID_COMPONENT_BUTT ) SetToolID( ID_COMPONENT_BUTT, wxCURSOR_PENCIL, _("Add Component"));
            OnLeftClick(DC, MousePos);
			break;

		case HK_BEGIN_WIRE:	// Add wire
 			if ( DrawStruct )	// An item is selected. If edited and not a wire, a new command is not possible
			{
				if ( DrawStruct->m_Flags )	// Item selected and edition in progress
				{
					if (DrawStruct->m_StructType == DRAW_SEGMENT_STRUCT_TYPE )
					{
						EDA_DrawLineStruct * segment = (EDA_DrawLineStruct *)DrawStruct;
						if ( segment->m_Layer != LAYER_WIRE ) break;
					}
					else break;
				}
			}
           // switch to m_ID_current_state = ID_WIRE_BUTT;
            if ( m_ID_current_state != ID_WIRE_BUTT ) SetToolID( ID_WIRE_BUTT, wxCURSOR_PENCIL, _("Add Wire"));
            OnLeftClick(DC, MousePos);
            break;

		case HK_ROTATE_COMPONENT:	// Component Rotation
			if ( DrawStruct == NULL )
			{
				DrawStruct = PickStruct( GetScreen()->m_Curseur,
					GetScreen()->EEDrawList, LIBITEM|TEXTITEM|LABELITEM );
				if ( DrawStruct == NULL ) break;
				if ( DrawStruct->m_StructType == DRAW_LIB_ITEM_STRUCT_TYPE )
						DrawStruct = LocateSmallestComponent( GetScreen() );
				if ( DrawStruct == NULL ) break;
			}
			switch (DrawStruct->m_StructType)
			{
				case DRAW_LIB_ITEM_STRUCT_TYPE:
					if ( DrawStruct->m_Flags == 0 )
					{
						SaveCopyInUndoList(DrawStruct, IS_CHANGED);
						RefreshToolBar = TRUE;
					}
						
					CmpRotationMiroir(
						(EDA_SchComponentStruct *) DrawStruct, DC, CMP_ROTATE_COUNTERCLOCKWISE );
					break;

				case DRAW_TEXT_STRUCT_TYPE:
				case DRAW_LABEL_STRUCT_TYPE:
				case DRAW_GLOBAL_LABEL_STRUCT_TYPE:
					if ( DrawStruct->m_Flags == 0 )
					{
						SaveCopyInUndoList(DrawStruct, IS_CHANGED);
						RefreshToolBar = TRUE;
					}
						ChangeTextOrient( (DrawTextStruct*)DrawStruct, DC);
						break;
			}
			break;

		case HK_MIRROR_Y_COMPONENT:	// Mirror Y (Component)
			if ( DrawStruct == NULL )
				DrawStruct = LocateSmallestComponent( GetScreen() );
			if ( DrawStruct )
			{
				if ( DrawStruct->m_Flags == 0 )
				{
					SaveCopyInUndoList(DrawStruct, IS_CHANGED);
					RefreshToolBar = TRUE;
				}
				CmpRotationMiroir(
					(EDA_SchComponentStruct *) DrawStruct, DC, CMP_MIROIR_Y );
			}
			break;

		case HK_MIRROR_X_COMPONENT:	// Mirror X (Component)
			if ( DrawStruct == NULL )
				DrawStruct = LocateSmallestComponent( GetScreen() );
			if ( DrawStruct )
			{
				if ( DrawStruct->m_Flags == 0 )
				{
					SaveCopyInUndoList(DrawStruct, IS_CHANGED);
					RefreshToolBar = TRUE;
				}
				CmpRotationMiroir(
					(EDA_SchComponentStruct *) DrawStruct, DC, CMP_MIROIR_X );
			}
			break;

 		case HK_ORIENT_NORMAL_COMPONENT:	// Orient 0, no mirror (Component)
			if ( DrawStruct == NULL )
				DrawStruct = LocateSmallestComponent( GetScreen() );
			if ( DrawStruct )
			{
				if ( DrawStruct->m_Flags == 0 )
				{
					SaveCopyInUndoList(DrawStruct, IS_CHANGED);
					RefreshToolBar = TRUE;
				}
				CmpRotationMiroir(
					(EDA_SchComponentStruct *) DrawStruct, DC, CMP_NORMAL );
				TestDanglingEnds(m_CurrentScreen->EEDrawList, DC);
			}
			break;

		case HK_MOVE_COMPONENT:	// Start move Component
			if ( PopupOn ) break;
			if ( DrawStruct == NULL )
				DrawStruct = LocateSmallestComponent( GetScreen() );
			if ( DrawStruct && (DrawStruct->m_Flags ==0) )
			{
				m_CurrentScreen->m_CurrentItem = DrawStruct;
				Process_Move_Item(m_CurrentScreen->m_CurrentItem, DC);
			}
			break;
	}
	
	if ( RefreshToolBar ) SetToolbars();
}


/***********************************************************/
void WinEDA_LibeditFrame::OnHotKey(wxDC * DC, int hotkey,
					EDA_BaseStruct * DrawStruct)
/***********************************************************/
/* Hot keys for the component editot. Some commands are relatives to the item under the mouse cursor
	Commands are case insensitive
	Zoom commands are not managed here
*/
{
bool PopupOn = m_CurrentScreen->m_CurrentItem  &&
			m_CurrentScreen->m_CurrentItem->m_Flags;

bool RefreshToolBar = FALSE;	// We must refresh tool bar when the undo/redo tool state is modified
	
	if ( hotkey == 0 ) return;

wxPoint MousePos = m_CurrentScreen->m_MousePosition; 

	/* Convert lower to upper case (the usual toupper function has problem with non ascii codes like function keys */
	if( (hotkey >= 'a') && (hotkey <= 'z') ) hotkey += 'A' - 'a';
	switch ( GetCommandCodeFromHotkey(hotkey, s_LibEdit_Hotkey_List)  )
	{
		default:
		case HK_NOT_FOUND:
			return;
			break;
		
		case HK_HELP:	// Display Current hotkey list
			DisplayHotkeyList(this, s_LibEdit_Hotkey_List);
			break;

		case HK_ZOOM_IN:
		case HK_ZOOM_OUT:
		case HK_ZOOM_REDRAW:
		case HK_ZOOM_CENTER:
			break;

		case HK_REPEAT_LAST:
			if ( LibItemToRepeat && (LibItemToRepeat->m_Flags == 0) &&
				 (LibItemToRepeat->m_StructType == COMPONENT_PIN_DRAW_TYPE) )
			{
				RepeatPinItem(DC, (LibDrawPin*) LibItemToRepeat);
			}
			else wxBell();
			break;
	}

	if ( RefreshToolBar ) SetToolbars();
}

