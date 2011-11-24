/**
 * @file gen_modules_placefile.cpp
 */

/*
 *  1 - create ascii files for automatic placement of smd components
 *  2 - create a module report (pos and module descr) (ascii file)
 */

#include "fctsys.h"
#include "confirm.h"
#include "kicad_string.h"
#include "gestfich.h"
#include "wxPcbStruct.h"
#include "trigo.h"
#include "appl_wxstruct.h"
#include "build_version.h"
#include "macros.h"

#include "class_board.h"
#include "class_module.h"
#include "class_drawsegment.h"

#include "pcbnew.h"


class LIST_MOD      /* Can list the elements of useful modules. */
{
public:
    MODULE*       m_Module;
    const wxChar* m_Reference;
    const wxChar* m_Value;
};


static wxPoint File_Place_Offset;  /* Offset coordinates for generated file. */

static void WriteDrawSegmentPcb( DRAWSEGMENT* PtDrawSegment, FILE* rptfile );


/* Sort function use by GenereModulesPosition() */
static int ListeModCmp( const void* o1, const void* o2 )
{
    LIST_MOD* ref = (LIST_MOD*) o1;
    LIST_MOD* cmp = (LIST_MOD*) o2;

    return StrNumCmp( ref->m_Reference, cmp->m_Reference, 16 );
}


#if defined(DEBUG)

/**
 * Function HasNonSMDPins
 * returns true if the given module has any non smd pins, such as through hole
 * and therefore cannot be placed automatically.
 */
static bool HasNonSMDPins( MODULE* aModule )
{
    D_PAD* pad;

    for( pad = aModule->m_Pads;  pad;  pad = pad->Next() )
    {
        if( pad->m_Attribut != PAD_SMD )
            return true;
    }

    return false;
}

#endif


void PCB_EDIT_FRAME::GenModulesPosition( wxCommandEvent& event )
{
    bool        doBoardBack = false;
    MODULE*     module;
    LIST_MOD*   list = NULL;
    char        line[1024];
    wxFileName  fnFront;
    wxFileName  fnBack;
    wxString    msg;
    wxString    frontLayerName;
    wxString    backLayerName;
    wxString    Title;
    FILE*       fpFront = 0;
    FILE*       fpBack = 0;
    bool        switchedLocale = false;

    /* Calculate conversion scales. */
    double conv_unit = 0.0001; /* unites = INCHES */

//  if(IF_DRILL_METRIC) conv_unit = 0.000254; /* unites = mm */

    File_Place_Offset = m_Auxiliary_Axis_Position;

    /* Calculating the number of useful modules (CMS attribute, not VIRTUAL) */
    int moduleCount = 0;

    for( module = GetBoard()->m_Modules;  module;  module = module->Next() )
    {
        if( module->m_Attributs & MOD_VIRTUAL )
        {
            D( printf( "skipping module %s because it's virtual\n",
                       TO_UTF8( module->GetReference() ) );)
            continue;
        }

        if( ( module->m_Attributs & MOD_CMS )  == 0 )
        {
#if 1 && defined(DEBUG)  // enable this code to fix a bunch of mis-labeled modules:
            if( !HasNonSMDPins( module ) )
            {
                // all module's pins are SMD, mark the part for pick and place
                module->m_Attributs |= MOD_CMS;
            }
            else
            {
                printf( "skipping %s because its attribute is not CMS and it has non SMD pins\n",
                        TO_UTF8(module->GetReference()) );
                continue;
            }
#else
            continue;
#endif
        }

        if( module->GetLayer() == LAYER_N_BACK )
            doBoardBack = true;

        moduleCount++;
    }

    if( moduleCount == 0 )
    {
        DisplayError( this, _( "No modules for automated placement." ) );
        return;
    }

    wxString boardFilePath = ( (wxFileName) GetScreen()->GetFileName()).GetPath();
    wxDirDialog dirDialog( this, _( "Select Output Directory" ), boardFilePath );

    if( dirDialog.ShowModal() == wxID_CANCEL )
        return;

    fnFront = GetScreen()->GetFileName();
    fnFront.SetPath( dirDialog.GetPath() );
    frontLayerName = GetBoard()->GetLayerName( LAYER_N_FRONT );
    fnFront.SetName( fnFront.GetName() + wxT( "_" ) + frontLayerName );
    fnFront.SetExt( wxT( "pos") );
    fpFront = wxFopen( fnFront.GetFullPath(), wxT( "wt" ) );

    if( fpFront == 0 )
    {
        msg = _( "Unable to create " ) + fnFront.GetFullPath();
        DisplayError( this, msg );
        goto exit;
    }

    if( doBoardBack )
    {
        fnBack = GetScreen()->GetFileName();
        fnBack.SetPath( dirDialog.GetPath() );
        backLayerName = GetBoard()->GetLayerName( LAYER_N_BACK );
        fnBack.SetName( fnBack.GetName() + wxT( "_" ) + backLayerName );
        fnBack.SetExt( wxT( "pos" ) );
        fpBack = wxFopen( fnBack.GetFullPath(), wxT( "wt" ) );

        if( fpBack == 0 )
        {
            msg = _( "Unable to create " ) + fnBack.GetFullPath();
            DisplayError( this, msg );
            goto exit;
        }
    }

    // Switch the locale to standard C (needed to print floating point
    // numbers like 1.3)
    SetLocaleTo_C_standard( );
    switchedLocale = true;

    // Display results
    ClearMsgPanel();
    AppendMsgPanel( _( "Component side place file:" ), fnFront.GetFullPath(), BLUE );

    if( doBoardBack )
        AppendMsgPanel( _( "Copper side place file:" ), fnBack.GetFullPath(), BLUE );

    msg.Empty(); msg << moduleCount;
    AppendMsgPanel( _( "Module count" ), msg, RED );

    // Sort the list of modules alphabetically
    list = new LIST_MOD[moduleCount];

    module = GetBoard()->m_Modules;

    for( int ii = 0;  module;  module = module->Next() )
    {
        if( module->m_Attributs & MOD_VIRTUAL )
            continue;

        if( (module->m_Attributs & MOD_CMS)  == 0 )
            continue;

        list[ii].m_Module    = module;
        list[ii].m_Reference = module->m_Reference->m_Text;
        list[ii].m_Value     = module->m_Value->m_Text;
        ii++;
    }

    qsort( list, moduleCount, sizeof(LIST_MOD), ListeModCmp );

    // Write file header
    sprintf( line, "### Module positions - created on %s ###\n", TO_UTF8( DateAndTime() ) );
    fputs( line, fpFront );

    if( doBoardBack )
        fputs( line, fpBack );

    Title = wxGetApp().GetAppName() + wxT( " " ) + GetBuildVersion();
    sprintf( line, "### Printed by Pcbnew version %s\n", TO_UTF8( Title ) );
    fputs( line, fpFront );

    if( doBoardBack )
        fputs( line, fpBack );

    sprintf( line, "## Unit = inches, Angle = deg.\n" );
    fputs( line, fpFront );

    if( doBoardBack )
        fputs( line, fpBack );

    sprintf( line, "## Side : %s\n", TO_UTF8( frontLayerName ) );
    fputs( line, fpFront );

    if( doBoardBack )
    {
        sprintf( line, "## Side : %s\n", TO_UTF8( backLayerName ) );
        fputs( line, fpBack );
    }

    sprintf( line,
             "# Ref    Val                  PosX       PosY        Rot     Side\n" );
    fputs( line, fpFront );

    if( doBoardBack )
        fputs( line, fpBack );

    for( int ii = 0; ii < moduleCount; ii++ )
    {
        wxPoint  module_pos;
        wxString ref = list[ii].m_Reference;
        wxString val = list[ii].m_Value;
        sprintf( line, "%-8.8s %-16.16s ", TO_UTF8( ref ), TO_UTF8( val ) );

        module_pos    = list[ii].m_Module->m_Pos;
        module_pos.x -= File_Place_Offset.x;
        module_pos.y -= File_Place_Offset.y;

        char* text = line + strlen( line );
        sprintf( text, " %9.4f  %9.4f  %8.1f    ",
                 module_pos.x * conv_unit,
                 module_pos.y * conv_unit,
                 double(list[ii].m_Module->m_Orient) / 10 );

        int layer = list[ii].m_Module->GetLayer();

        wxASSERT( layer==LAYER_N_FRONT || layer==LAYER_N_BACK );

        if( layer == LAYER_N_FRONT )
        {
            strcat( line, TO_UTF8( frontLayerName ) );
            strcat( line, "\n" );
            fputs( line, fpFront );
        }
        else if( layer == LAYER_N_BACK )
        {
            strcat( line, TO_UTF8( backLayerName ) );
            strcat( line, "\n" );
            fputs( line, fpBack );
        }
    }

    // Write EOF
    fputs( "## End\n", fpFront );

    if( doBoardBack )
        fputs( "## End\n", fpBack );

    msg = _( "Module position files created:" );
    msg.Append( wxT( "\n\n" ) + frontLayerName + wxT( ":\n" ) );
    msg.Append( fnFront.GetFullPath() );

    if( doBoardBack )
    {
        msg.Append( wxT( "\n\n" ) + backLayerName + wxT( ":\n" ) );
        msg.Append( fnBack.GetFullPath() );
    }

    wxMessageBox( msg, _( "Module Position File" ), wxICON_INFORMATION );

exit:   // the only safe way out of here, no returns please.

    if( list )
        delete[] list;

    if( switchedLocale )
        SetLocaleTo_Default( );      // revert to the current locale

    if( fpFront )
        fclose( fpFront );

    if( fpBack )
        fclose( fpBack );
}


/* Print a module report.
 */
void PCB_EDIT_FRAME::GenModuleReport( wxCommandEvent& event )
{
    double   conv_unit;
    MODULE*  Module;
    D_PAD*   pad;
    char     line[1024];
    wxFileName fn;
    wxString fnFront, msg;
    FILE*    rptfile;
    wxPoint  module_pos;

    conv_unit = 0.0001; /* unites = INCHES */

//  if(IF_DRILL_METRIC) conv_unit = 0.000254; /* unites = mm */

    File_Place_Offset = wxPoint( 0, 0 );

    wxString boardFilePath = ( (wxFileName) GetScreen()->GetFileName()).GetPath();
    wxDirDialog dirDialog( this, _( "Select Output Directory" ), boardFilePath );

    if( dirDialog.ShowModal() == wxID_CANCEL )
        return;

    fn = GetScreen()->GetFileName();
    fn.SetPath( dirDialog.GetPath() );
    fn.SetExt( wxT( "rpt" ) );

    rptfile = wxFopen( fn.GetFullPath(), wxT( "wt" ) );

    if( rptfile == NULL )
    {
        msg = _( "Unable to create " ) + fn.GetFullPath();
        DisplayError( this, msg );
        return;
    }

    // Switch the locale to standard C (needed to print floating point
    // numbers like 1.3)
    SetLocaleTo_C_standard();

    /* Generate header file comments.) */
    sprintf( line, "## Module report - date %s\n", TO_UTF8( DateAndTime() ) );
    fputs( line, rptfile );

    wxString Title = wxGetApp().GetAppName() + wxT( " " ) + GetBuildVersion();
    sprintf( line, "## Created by Pcbnew version %s\n", TO_UTF8( Title ) );
    fputs( line, rptfile );
    fputs( "## Unit = inches, Angle = deg.\n", rptfile );

    fputs( "##\n", rptfile );
    fputs( "\n$BeginDESCRIPTION\n", rptfile );

    GetBoard()->ComputeBoundingBox();
    fputs( "\n$BOARD\n", rptfile );
    fputs( "unit INCH\n", rptfile );
    sprintf( line, "upper_left_corner %9.6f %9.6f\n",
             GetBoard()->m_BoundaryBox.GetX() * conv_unit,
             GetBoard()->m_BoundaryBox.GetY() * conv_unit );
    fputs( line, rptfile );

    sprintf( line, "lower_right_corner %9.6f %9.6f\n",
             GetBoard()->m_BoundaryBox.GetRight() * conv_unit,
             GetBoard()->m_BoundaryBox.GetBottom() * conv_unit );
    fputs( line, rptfile );

    fputs( "$EndBOARD\n\n", rptfile );

    Module = (MODULE*) GetBoard()->m_Modules;

    for( ; Module != NULL; Module = Module->Next() )
    {
        sprintf( line, "$MODULE %s\n", EscapedUTF8( Module->m_Reference->m_Text ).c_str() );
        fputs( line, rptfile );

        sprintf( line, "reference %s\n", EscapedUTF8( Module->m_Reference->m_Text ).c_str() );
        fputs( line, rptfile );
        sprintf( line, "value %s\n", EscapedUTF8( Module->m_Value->m_Text ).c_str() );
        fputs( line, rptfile );
        sprintf( line, "footprint %s\n", EscapedUTF8( Module->m_LibRef ).c_str() );
        fputs( line, rptfile );

        msg = wxT( "attribut" );

        if( Module->m_Attributs & MOD_VIRTUAL )
            msg += wxT( " virtual" );

        if( Module->m_Attributs & MOD_CMS )
            msg += wxT( " smd" );

        if( ( Module->m_Attributs & (MOD_VIRTUAL | MOD_CMS) ) == 0 )
            msg += wxT( " none" );

        msg += wxT( "\n" );
        fputs( TO_UTF8( msg ), rptfile );

        module_pos    = Module->m_Pos;
        module_pos.x -= File_Place_Offset.x;
        module_pos.y -= File_Place_Offset.y;

        sprintf( line, "position %9.6f %9.6f\n",
                 module_pos.x * conv_unit,
                 module_pos.y * conv_unit );
        fputs( line, rptfile );

        sprintf( line, "orientation  %.2f\n", (double) Module->m_Orient / 10 );

        if( Module->GetLayer() == LAYER_N_FRONT )
            strcat( line, "layer component\n" );
        else if( Module->GetLayer() == LAYER_N_BACK )
            strcat( line, "layer copper\n" );
        else
            strcat( line, "layer other\n" );

        fputs( line, rptfile );

        Module->Write_3D_Descr( rptfile );

        for( pad = Module->m_Pads; pad != NULL; pad = pad->Next() )
        {
            fprintf( rptfile, "$PAD \"%.4s\"\n", pad->m_Padname );
            sprintf( line, "position %9.6f %9.6f\n",
                     pad->m_Pos0.x * conv_unit,
                     pad->m_Pos0.y * conv_unit );
            fputs( line, rptfile );

            sprintf( line, "size %9.6f %9.6f\n",
                     pad->m_Size.x * conv_unit,
                     pad->m_Size.y * conv_unit );
            fputs( line, rptfile );
            sprintf( line, "drill %9.6f\n", pad->m_Drill.x * conv_unit );
            fputs( line, rptfile );
            sprintf( line, "shape_offset %9.6f %9.6f\n",
                     pad->m_Offset.x * conv_unit,
                     pad->m_Offset.y * conv_unit );
            fputs( line, rptfile );

            sprintf( line, "orientation  %.2f\n",
                     double(pad->m_Orient - Module->m_Orient) / 10 );
            fputs( line, rptfile );
            const char* shape_name[6] = { "??? ", "Circ", "Rect", "Oval", "trap", "spec" };
            sprintf( line, "Shape  %s\n", shape_name[pad->m_PadShape] );
            fputs( line, rptfile );

            int layer = 0;

            if( pad->m_layerMask & LAYER_BACK )
                layer = 1;

            if( pad->m_layerMask & LAYER_FRONT )
                layer |= 2;

            const char* layer_name[4] = { "??? ", "copper", "component", "all" };
            sprintf( line, "Layer  %s\n", layer_name[layer] );
            fputs( line, rptfile );
            fprintf( rptfile, "$EndPAD\n" );
        }

        fprintf( rptfile, "$EndMODULE  %s\n\n",
                 TO_UTF8(Module->m_Reference->m_Text ) );
    }

    /* Write board Edges */
    EDA_ITEM* PtStruct;

    for( PtStruct = GetBoard()->m_Drawings; PtStruct != NULL; PtStruct = PtStruct->Next() )
    {
        if( PtStruct->Type() != PCB_LINE_T )
            continue;

        if( ( (DRAWSEGMENT*) PtStruct )->GetLayer() != EDGE_N )
            continue;

        WriteDrawSegmentPcb( (DRAWSEGMENT*) PtStruct, rptfile );
    }

    /* Generate EOF. */
    fputs( "$EndDESCRIPTION\n", rptfile );
    fclose( rptfile );
    SetLocaleTo_Default( );      // revert to the current locale

    msg = _( "Module report file created:" );
    msg.Append( wxT( "\n" ) + fn.GetFullPath() );

    wxMessageBox( msg, _( "Module Report" ), wxICON_INFORMATION );
}


/* Output to rpt file a segment type from the PCB drawing.
 * The contours are of different types:
 * Segment
 * Circle
 * Arc
 */
void WriteDrawSegmentPcb( DRAWSEGMENT* PtDrawSegment, FILE* rptfile )
{
    double conv_unit, ux0, uy0, dx, dy;
    double radius, width;
    char   line[1024];

    conv_unit = 0.0001; /* units = INCHES */

    ux0 = PtDrawSegment->m_Start.x * conv_unit;
    uy0 = PtDrawSegment->m_Start.y * conv_unit;

    dx = PtDrawSegment->m_End.x * conv_unit;
    dy = PtDrawSegment->m_End.y * conv_unit;

    width = PtDrawSegment->m_Width * conv_unit;

    switch( PtDrawSegment->m_Shape )
    {
    case S_CIRCLE:
        radius = hypot( dx - ux0, dy - uy0 );
        fprintf( rptfile, "$CIRCLE \n" );
        fprintf( rptfile, "centre %.6lf %.6lf\n", ux0, uy0 );
        fprintf( rptfile, "radius %.6lf\n", radius );
        fprintf( rptfile, "width %.6lf\n", width );
        fprintf( rptfile, "$EndCIRCLE \n" );
        break;

    case S_ARC:
        {
            int endx = PtDrawSegment->m_End.x, endy = PtDrawSegment->m_End.y;
            radius = hypot( dx - ux0, dy - uy0 );
            RotatePoint( &endx,
                         &endy,
                         PtDrawSegment->m_Start.x,
                         PtDrawSegment->m_Start.y,
                         PtDrawSegment->m_Angle );

            fprintf( rptfile, "$ARC \n" );
            fprintf( rptfile, "centre %.6lf %.6lf\n", ux0, uy0 );
            fprintf( rptfile, "start %.6lf %.6lf\n",
                     endx * conv_unit, endy * conv_unit );
            fprintf( rptfile, "end %.6lf %.6lf\n", dx, dy );
            fprintf( rptfile, "width %.6lf\n", width );
            fprintf( rptfile, "$EndARC \n" );
        }
        break;

    default:
        sprintf( line, "$LINE \n" );
        fputs( line, rptfile );

        fprintf( rptfile, "start %.6lf %.6lf\n", ux0, uy0 );
        fprintf( rptfile, "end %.6lf %.6lf\n", dx, dy );
        fprintf( rptfile, "width %.6lf\n", width );
        fprintf( rptfile, "$EndLINE \n" );
        break;
    }
}
