/**
 * @file pcbnew/hotkeys.h
 * PCBNew hotkeys
 */
#ifndef _PCBNEW_KOTKEYS_H
#define _PCBNEW_HOTKEYS_H

#include "hotkeys_basic.h"

// List of hot keys id.
// see also enum common_hotkey_id_commnand in hotkeys_basic.h
// for shared hotkeys id
enum hotkey_id_commnand {
    HK_DELETE = HK_COMMON_END,
    HK_BACK_SPACE,
    HK_ROTATE_ITEM,
    HK_MOVE_ITEM,
    HK_DRAG_ITEM,
    HK_FLIP_FOOTPRINT,
    HK_GET_AND_MOVE_FOOTPRINT,
    HK_LOCK_UNLOCK_FOOTPRINT,
    HK_ADD_NEW_TRACK,
    HK_ADD_VIA,
    HK_ADD_MICROVIA,
    HK_SWITCH_TRACK_POSTURE,
    HK_DRAG_TRACK_KEEP_SLOPE,
    HK_END_TRACK,
    HK_SAVE_BOARD, HK_LOAD_BOARD,
    HK_SWITCH_UNITS,
    HK_SWITCH_TRACK_DISPLAY_MODE,
    HK_FIND_ITEM,
    HK_EDIT_ITEM,
    HK_PLACE_ITEM,
    HK_SWITCH_TRACK_WIDTH_TO_NEXT,
    HK_SWITCH_TRACK_WIDTH_TO_PREVIOUS,
    HK_SWITCH_GRID_TO_FASTGRID1,
    HK_SWITCH_GRID_TO_FASTGRID2,
    HK_SWITCH_GRID_TO_NEXT,
    HK_SWITCH_GRID_TO_PREVIOUS,
    HK_SWITCH_LAYER_TO_COPPER,
    HK_SWITCH_LAYER_TO_COMPONENT,
    HK_SWITCH_LAYER_TO_NEXT,
    HK_SWITCH_LAYER_TO_PREVIOUS,
    HK_SWITCH_LAYER_TO_INNER1,
    HK_SWITCH_LAYER_TO_INNER2,
    HK_SWITCH_LAYER_TO_INNER3,
    HK_SWITCH_LAYER_TO_INNER4,
    HK_SWITCH_LAYER_TO_INNER5,
    HK_SWITCH_LAYER_TO_INNER6,
    HK_SWITCH_LAYER_TO_INNER7,
    HK_SWITCH_LAYER_TO_INNER8,
    HK_SWITCH_LAYER_TO_INNER9,
    HK_SWITCH_LAYER_TO_INNER10,
    HK_SWITCH_LAYER_TO_INNER11,
    HK_SWITCH_LAYER_TO_INNER12,
    HK_SWITCH_LAYER_TO_INNER13,
    HK_SWITCH_LAYER_TO_INNER14,
    HK_ADD_MODULE,
    HK_SLIDE_TRACK,
    HK_RECORD_MACROS_0,
    HK_CALL_MACROS_0,
    HK_RECORD_MACROS_1,
    HK_CALL_MACROS_1,
    HK_RECORD_MACROS_2,
    HK_CALL_MACROS_2,
    HK_RECORD_MACROS_3,
    HK_CALL_MACROS_3,
    HK_RECORD_MACROS_4,
    HK_CALL_MACROS_4,
    HK_RECORD_MACROS_5,
    HK_CALL_MACROS_5,
    HK_RECORD_MACROS_6,
    HK_CALL_MACROS_6,
    HK_RECORD_MACROS_7,
    HK_CALL_MACROS_7,
    HK_RECORD_MACROS_8,
    HK_CALL_MACROS_8,
    HK_RECORD_MACROS_9,
    HK_CALL_MACROS_9
};

// Full list of hotkey descriptors for board editor and footprint editor
extern struct EDA_HOTKEY_CONFIG g_Pcbnew_Editor_Hokeys_Descr[];

// List of hotkey descriptors for the board editor only
extern struct EDA_HOTKEY_CONFIG g_Board_Editor_Hokeys_Descr[];

// List of hotkey descriptors for the footprint editor only
extern struct EDA_HOTKEY_CONFIG g_Module_Editor_Hokeys_Descr[];

// List of common hotkey descriptors
// used in hotkeys_board_editor.cpp and hotkeys_module_editor.cpp
extern EDA_HOTKEY* common_Hotkey_List[];

// List of hotkey descriptors for pcbnew
// used in hotkeys_board_editor.cpp
extern EDA_HOTKEY* board_edit_Hotkey_List[];

// List of hotkey descriptors for the module editor
// used in hotkeys_module_editor.cpp
extern EDA_HOTKEY* module_edit_Hotkey_List[];


#endif /* _PCBNEW_HOTKEYS_H_ */
