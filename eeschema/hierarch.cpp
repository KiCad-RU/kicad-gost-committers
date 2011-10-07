/**
 * @file hierarch.cpp
 */

#include "fctsys.h"
#include "class_drawpanel.h"
#include "confirm.h"
#include "wxEeschemaStruct.h"

#include "general.h"
#include "sch_sheet.h"
#include "sch_sheet_path.h"

#include "wx/imaglist.h"
#include "wx/treectrl.h"


enum
{
    ID_TREECTRL_HIERARCHY = 1600
};


class HIERARCHY_NAVIG_DLG;

/* This class derived from wxTreeItemData stores the SCH_SHEET_PATH of each
 * sheet in hierarchy in each TreeItem, in its associated data buffer
*/
class TreeItemData : public wxTreeItemData
{
public:
    SCH_SHEET_PATH m_SheetPath;
    TreeItemData( SCH_SHEET_PATH sheet ) : wxTreeItemData()
    {
        m_SheetPath = sheet;
    }
};

/* Class to handle hierarchy tree. */
class HIERARCHY_TREE : public wxTreeCtrl
{
private:
    HIERARCHY_NAVIG_DLG* m_Parent;
    wxImageList*      imageList;

public:
    HIERARCHY_TREE() { }
    HIERARCHY_TREE( HIERARCHY_NAVIG_DLG* parent );

    DECLARE_DYNAMIC_CLASS( HIERARCHY_TREE )
};

IMPLEMENT_DYNAMIC_CLASS( HIERARCHY_TREE, wxTreeCtrl )


HIERARCHY_TREE::HIERARCHY_TREE( HIERARCHY_NAVIG_DLG* parent ) :
    wxTreeCtrl( (wxWindow*)parent, ID_TREECTRL_HIERARCHY, wxDefaultPosition, wxDefaultSize,
                wxTR_HAS_BUTTONS, wxDefaultValidator, wxT( "HierachyTreeCtrl" ) )
{
    m_Parent = parent;

    // Make an image list containing small icons
    imageList = new wxImageList( 16, 15, true, 2 );

    imageList->Add( KiBitmap( tree_nosel_xpm ) );
    imageList->Add( KiBitmap( tree_sel_xpm ) );

    AssignImageList( imageList );
}


class HIERARCHY_NAVIG_DLG : public wxDialog
{
public:
    SCH_EDIT_FRAME* m_Parent;
    HIERARCHY_TREE*    m_Tree;
    int             m_nbsheets;
    wxDC*           m_DC;

private:
    wxSize m_TreeSize;
    int    maxposx;

public:
    HIERARCHY_NAVIG_DLG( SCH_EDIT_FRAME* parent, wxDC* DC, const wxPoint& pos );
    void BuildSheetsTree( SCH_SHEET_PATH* list, wxTreeItemId* previousmenu );

    ~HIERARCHY_NAVIG_DLG();

    void OnSelect( wxTreeEvent& event );

private:
    void OnQuit( wxCommandEvent& event );

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE( HIERARCHY_NAVIG_DLG, wxDialog )
    EVT_TREE_ITEM_ACTIVATED( ID_TREECTRL_HIERARCHY, HIERARCHY_NAVIG_DLG::OnSelect )
END_EVENT_TABLE()


void SCH_EDIT_FRAME::InstallHierarchyFrame( wxDC* DC, wxPoint& pos )
{
    HIERARCHY_NAVIG_DLG* treeframe = new HIERARCHY_NAVIG_DLG( this, DC, pos );

    treeframe->ShowModal();
    treeframe->Destroy();
}


HIERARCHY_NAVIG_DLG::HIERARCHY_NAVIG_DLG( SCH_EDIT_FRAME* parent, wxDC* DC, const wxPoint& pos ) :
    wxDialog( parent, -1, _( "Navigator" ), pos, wxSize( 110, 50 ), DIALOG_STYLE )
{
    wxTreeItemId cellule;

    m_Parent = parent;
    m_DC   = DC;
    m_Tree = new HIERARCHY_TREE( this );

    m_nbsheets = 1;

    cellule = m_Tree->AddRoot( _( "Root" ), 0, 1 );
    m_Tree->SetItemBold( cellule, true );
    SCH_SHEET_PATH list;
    list.Push( g_RootSheet );
    m_Tree->SetItemData( cellule, new TreeItemData( list ) );

    wxRect itemrect;
#ifdef __UNIX__
    itemrect.SetWidth( 100 );
    itemrect.SetHeight( 20 );
#else
    m_Tree->GetBoundingRect( cellule, itemrect );
#endif
    m_TreeSize.x = itemrect.GetWidth() + 10;
    m_TreeSize.y = 20;

    if( m_Parent->GetSheet()->Last() == g_RootSheet )
        m_Tree->SelectItem( cellule ); //root.

    maxposx = 15;
    BuildSheetsTree( &list, &cellule );

    if( m_nbsheets > 1 )
    {
        m_Tree->Expand( cellule );

        // Readjust the size of the frame to an optimal value.
        m_TreeSize.y += m_nbsheets * itemrect.GetHeight();
        m_TreeSize.x  = MIN( m_TreeSize.x, 250 );
        m_TreeSize.y  = MIN( m_TreeSize.y, 350 );
        SetClientSize( m_TreeSize );
    }
}


HIERARCHY_NAVIG_DLG::~HIERARCHY_NAVIG_DLG()
{
}


void HIERARCHY_NAVIG_DLG::OnQuit( wxCommandEvent& event )
{
    // true is to force the frame to close
    Close( true );
}


/* Routine to create the hierarchical tree of the schematic
 * This routine is re-entrant!
 */
void HIERARCHY_NAVIG_DLG::BuildSheetsTree( SCH_SHEET_PATH* list, wxTreeItemId*  previousmenu )

{
    wxTreeItemId menu;

    if( m_nbsheets > NB_MAX_SHEET )
    {
        if( m_nbsheets == (NB_MAX_SHEET + 1) )
        {
            wxString msg;
            msg << wxT( "BuildSheetsTree: Error: nbsheets > " ) << NB_MAX_SHEET;
            DisplayError( this, msg );
            m_nbsheets++;
        }

        return;
    }

    maxposx += m_Tree->GetIndent();
    SCH_ITEM* schitem = list->LastDrawList();

    while( schitem && m_nbsheets < NB_MAX_SHEET )
    {
        if( schitem->Type() == SCH_SHEET_T )
        {
            SCH_SHEET* sheet = (SCH_SHEET*) schitem;
            m_nbsheets++;
            menu = m_Tree->AppendItem( *previousmenu, sheet->m_SheetName, 0, 1 );
            list->Push( sheet );
            m_Tree->SetItemData( menu, new TreeItemData( *list ) );
            int ll = m_Tree->GetItemText( menu ).Len();

#ifdef __WINDOWS__
            ll *= 9;    //  * char width
#else
            ll *= 12;   //  * char width
#endif
            ll += maxposx + 20;
            m_TreeSize.x  = MAX( m_TreeSize.x, ll );
            m_TreeSize.y += 1;

            if( *list == *( m_Parent->GetSheet() ) )
            {
                m_Tree->EnsureVisible( menu );
                m_Tree->SelectItem( menu );
            }

            BuildSheetsTree( list, &menu );
            m_Tree->Expand( menu );
            list->Pop();
        }

        schitem = schitem->Next();
    }

    maxposx -= m_Tree->GetIndent();
}


/* Called on a double-click on a tree item:
 * Open the selected sheet, and display the corresponding screen
 */
void HIERARCHY_NAVIG_DLG::OnSelect( wxTreeEvent& event )

{
    wxTreeItemId ItemSel = m_Tree->GetSelection();

    *(m_Parent->m_CurrentSheet) =
        ( (TreeItemData*) m_Tree->GetItemData( ItemSel ) )->m_SheetPath;
    m_Parent->DisplayCurrentSheet();
    Close( true );
}


void SCH_EDIT_FRAME::DisplayCurrentSheet()
{
    m_itemToRepeat = NULL;
    ClearMsgPanel();

    SCH_SCREEN* screen = m_CurrentSheet->LastScreen();

    SetScreen( screen );

    // update the References
    m_CurrentSheet->UpdateAllScreenReferences();
    SetSheetNumberAndCount();
    DrawPanel->m_CanStartBlock = -1;

    if( screen->m_FirstRedraw )
    {
        Zoom_Automatique( false );
        screen->m_FirstRedraw = false;
        screen->SetCrossHairPosition( screen->GetScrollCenterPosition() );
        DrawPanel->MoveCursorToCrossHair();
    }
    else
    {
        RedrawScreen( screen->GetScrollCenterPosition(), true );
    }

    // Now refresh DrawPanel. Should be not necessary, but because screen has changed
    // the previous refresh has set all new draw parameters (scroll position ..)
    // but most of time there were some inconsitencies about cursor parameters
    // ( previous position of cursor ...) and artefacts can happen
    // mainly when sheet size has changed
    // This second refresh clears artefacts because at this point,
    // all parameters are now updated
    DrawPanel->Refresh();
}
