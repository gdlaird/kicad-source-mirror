/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2012 Wayne Stambaugh <stambaughw@gmail.com>
 * Copyright (C) 1992-2018 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <fctsys.h>
#include <kiface_i.h>
#include <help_common_strings.h>
#include <dialog_helpers.h>
#include <pcb_edit_frame.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <bitmaps.h>
#include <class_board.h>
#include <tool/actions.h>
#include <pcbnew.h>
#include <pcbnew_id.h>
#include <hotkeys.h>
#include <pcb_layer_box_selector.h>
#include <view/view.h>

#include <wx/wupdlock.h>
#include <memory>
#include <pgm_base.h>
#include <tools/pcb_actions.h>

extern bool IsWxPythonLoaded();

#define SEL_LAYER_HELP _( \
        "Show active layer selections\nand select layer pair for route and place via" )


/* Data to build the layer pair indicator button */
static std::unique_ptr<wxBitmap> LayerPairBitmap;

#define BM_LAYERICON_SIZE 24
static const char s_BitmapLayerIcon[BM_LAYERICON_SIZE][BM_LAYERICON_SIZE] =
{
    // 0 = draw pixel with active layer color
    // 1 = draw pixel with top layer color (top/bottom layer used inautoroute and place via)
    // 2 = draw pixel with bottom layer color
    // 3 = draw pixel with via color
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 1, 1, 1, 1, 3, 0, 0, 0, 0, 0, 0, 0 },
    { 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 0, 1, 1, 1, 1, 3, 3, 2, 2, 2, 2, 2, 2, 2 },
    { 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 1, 1, 1, 1, 0, 3, 3, 2, 2, 2, 2, 2, 2, 2 },
    { 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 1, 1, 1, 1, 0, 3, 3, 2, 2, 2, 2, 2, 2, 2 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 1, 1, 1, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};


void PCB_EDIT_FRAME::PrepareLayerIndicator()
{
    int        ii, jj;
    COLOR4D    active_layer_color, Route_Layer_TOP_color,
               Route_Layer_BOTTOM_color, via_color, background_color;
    bool       change = false;

    static int previous_requested_scale;
    static COLOR4D previous_active_layer_color, previous_Route_Layer_TOP_color,
                   previous_Route_Layer_BOTTOM_color, previous_via_color,
                   previous_background_color;

    int requested_scale;
    Pgm().CommonSettings()->Read( ICON_SCALE_KEY, &requested_scale, 0 );

    if( requested_scale != previous_requested_scale )
    {
        previous_requested_scale = requested_scale;
        change = true;
    }

    active_layer_color = Settings().Colors().GetLayerColor(GetActiveLayer());

    if( previous_active_layer_color != active_layer_color )
    {
        previous_active_layer_color = active_layer_color;
        change = true;
    }

    Route_Layer_TOP_color =
        Settings().Colors().GetLayerColor( GetScreen()->m_Route_Layer_TOP );

    if( previous_Route_Layer_TOP_color != Route_Layer_TOP_color )
    {
        previous_Route_Layer_TOP_color = Route_Layer_TOP_color;
        change = true;
    }

    Route_Layer_BOTTOM_color =
        Settings().Colors().GetLayerColor( GetScreen()->m_Route_Layer_BOTTOM );

    if( previous_Route_Layer_BOTTOM_color != Route_Layer_BOTTOM_color )
    {
        previous_Route_Layer_BOTTOM_color = Route_Layer_BOTTOM_color;
        change = true;
    }

    int via_type = GetDesignSettings().m_CurrentViaType;
    via_color = Settings().Colors().GetItemColor( LAYER_VIAS + via_type );

    if( previous_via_color != via_color )
    {
        previous_via_color = via_color;
        change = true;
    }

    background_color = Settings().Colors().GetItemColor( LAYER_PCB_BACKGROUND );

    if( previous_background_color != background_color )
    {
        previous_background_color = background_color;
        change = true;
    }

    if( !change && LayerPairBitmap )
        return;

    LayerPairBitmap = std::make_unique<wxBitmap>( 24, 24 );

    /* Draw the icon, with colors according to the active layer and layer
     * pairs for via command (change layer)
     */
    wxMemoryDC iconDC;
    iconDC.SelectObject( *LayerPairBitmap );
    wxBrush    brush;
    wxPen      pen;
    int buttonColor = -1;

    brush.SetStyle( wxBRUSHSTYLE_SOLID );
    brush.SetColour( background_color.WithAlpha(1.0).ToColour() );
    iconDC.SetBrush( brush );
    iconDC.DrawRectangle( 0, 0, BM_LAYERICON_SIZE, BM_LAYERICON_SIZE );

    for( ii = 0; ii < BM_LAYERICON_SIZE; ii++ )
    {
        for( jj = 0; jj < BM_LAYERICON_SIZE; jj++ )
        {
            if( s_BitmapLayerIcon[ii][jj] != buttonColor )
            {
                switch( s_BitmapLayerIcon[ii][jj] )
                {
                default:
                case 0:
                    pen.SetColour( active_layer_color.ToColour() );
                    break;

                case 1:
                    pen.SetColour( Route_Layer_TOP_color.ToColour() );
                    break;

                case 2:
                    pen.SetColour( Route_Layer_BOTTOM_color.ToColour() );
                    break;

                case 3:
                    pen.SetColour( via_color.ToColour() );
                    break;
                }

                buttonColor = s_BitmapLayerIcon[ii][jj];
                iconDC.SetPen( pen );
            }

            iconDC.DrawPoint( jj, ii );
        }
    }

    /* Deselect the Tool Bitmap from DC,
     *  in order to delete the MemoryDC safely without deleting the bitmap */
    iconDC.SelectObject( wxNullBitmap );

    // Scale the bitmap
    const int scale = ( requested_scale <= 0 ) ? KiIconScale( this ) : requested_scale;
    wxImage image = LayerPairBitmap->ConvertToImage();

    // "NEAREST" causes less mixing of colors
    image.Rescale( scale * image.GetWidth() / 4, scale * image.GetHeight() / 4,
                   wxIMAGE_QUALITY_NEAREST );

    LayerPairBitmap = std::make_unique<wxBitmap>( image );

    if( m_mainToolBar )
    {
        m_mainToolBar->SetToolBitmap( ID_AUX_TOOLBAR_PCB_SELECT_LAYER_PAIR, *LayerPairBitmap );
        m_mainToolBar->Refresh();
    }
}


void PCB_EDIT_FRAME::ReCreateHToolbar()
{
    // Note:
    // To rebuild the aui toolbar, the more easy way is to clear ( calling m_mainToolBar.Clear() )
    // all wxAuiToolBarItems.
    // However the wxAuiToolBarItems are not the owners of controls managed by
    // them and therefore do not delete them
    // So we do not recreate them after clearing the tools.

    wxString msg;

    wxWindowUpdateLocker dummy( this );

    if( m_mainToolBar )
        m_mainToolBar->Clear();
    else
        m_mainToolBar = new ACTION_TOOLBAR( this, ID_H_TOOLBAR, wxDefaultPosition, wxDefaultSize,
                                            KICAD_AUI_TB_STYLE | wxAUI_TB_HORZ_LAYOUT );

#define ADD_TOOL( id, xpm, tooltip ) \
    m_mainToolBar->AddTool( id, wxEmptyString, KiScaledBitmap( xpm, this ), tooltip );

    // Set up toolbar
    if( Kiface().IsSingle() )
    {
        ADD_TOOL( ID_NEW_BOARD, new_board_xpm, _( "New board" ) );
        ADD_TOOL( ID_LOAD_FILE, open_brd_file_xpm, _( "Open existing board" ) );
    }

    ADD_TOOL( ID_SAVE_BOARD, save_xpm, _( "Save board" ) );

    KiScaledSeparator( m_mainToolBar, this );
    ADD_TOOL( ID_BOARD_SETUP_DIALOG, options_board_xpm, _( "Board setup" ) );

    KiScaledSeparator( m_mainToolBar, this );
    ADD_TOOL( ID_SHEET_SET, sheetset_xpm, _( "Page settings for paper size and texts" ) );
    ADD_TOOL( wxID_PRINT, print_button_xpm, _( "Print board" ) );
    ADD_TOOL( ID_GEN_PLOT, plot_xpm, _( "Plot (HPGL, PostScript, or GERBER format)" ) );

    KiScaledSeparator( m_mainToolBar, this );
    m_mainToolBar->Add( ACTIONS::undo );
    m_mainToolBar->Add( ACTIONS::redo );

    KiScaledSeparator( m_mainToolBar, this );
    m_mainToolBar->Add( ACTIONS::find );

    KiScaledSeparator( m_mainToolBar, this );
    m_mainToolBar->Add( ACTIONS::zoomRedraw );
    m_mainToolBar->Add( ACTIONS::zoomInCenter );
    m_mainToolBar->Add( ACTIONS::zoomOutCenter );
    m_mainToolBar->Add( ACTIONS::zoomFitScreen );
    m_mainToolBar->Add( ACTIONS::zoomTool, ACTION_TOOLBAR::TOGGLE );

    KiScaledSeparator( m_mainToolBar, this );
    ADD_TOOL( ID_OPEN_MODULE_EDITOR, module_editor_xpm, _( "Open footprint editor" ) );
    ADD_TOOL( ID_OPEN_MODULE_VIEWER, modview_icon_xpm, _( "Open footprint viewer" ) );

    KiScaledSeparator( m_mainToolBar, this );
    ADD_TOOL( ID_UPDATE_PCB_FROM_SCH, update_pcb_from_sch_xpm, _( "Update PCB from schematic" ) );
    ADD_TOOL( ID_DRC_CONTROL, erc_xpm, _( "Perform design rules check" ) );

    KiScaledSeparator( m_mainToolBar, this );

    if( m_SelLayerBox == nullptr )
    {
        m_SelLayerBox = new PCB_LAYER_BOX_SELECTOR( m_mainToolBar, ID_TOOLBARH_PCB_SELECT_LAYER );
        m_SelLayerBox->SetBoardFrame( this );
    }

    ReCreateLayerBox( false );
    m_mainToolBar->AddControl( m_SelLayerBox );

    PrepareLayerIndicator();    // Initialize the bitmap with the active layer colors
    m_mainToolBar->AddTool( ID_AUX_TOOLBAR_PCB_SELECT_LAYER_PAIR, wxEmptyString, *LayerPairBitmap,
                            SEL_LAYER_HELP );

    KiScaledSeparator( m_mainToolBar, this );
    ADD_TOOL( ID_RUN_EESCHEMA, eeschema_xpm, _( "Open schematic in Eeschema" ) );

    // Access to the scripting console
#if defined(KICAD_SCRIPTING_WXPYTHON)
    if( IsWxPythonLoaded() )
    {
        KiScaledSeparator( m_mainToolBar, this );

        m_mainToolBar->AddTool( ID_TOOLBARH_PCB_SCRIPTING_CONSOLE, wxEmptyString,
                                KiScaledBitmap( py_script_xpm, this ),
                                _( "Show/Hide the Python Scripting console" ), wxITEM_CHECK );

#if defined(KICAD_SCRIPTING) && defined(KICAD_SCRIPTING_ACTION_MENU)
        AddActionPluginTools();
#endif
    }
#endif

    // after adding the buttons to the toolbar, must call Realize() to reflect the changes
    m_mainToolBar->Realize();

#undef ADD_TOOL
}


void PCB_EDIT_FRAME::ReCreateOptToolbar()
{
    // Note:
    // To rebuild the aui toolbar, the more easy way is to clear ( calling m_mainToolBar.Clear() )
    // all wxAuiToolBarItems.
    // However the wxAuiToolBarItems are not the owners of controls managed by
    // them and therefore do not delete them
    // So we do not recreate them after clearing the tools.

    wxWindowUpdateLocker dummy( this );

    if( m_optionsToolBar )
        m_optionsToolBar->Clear();
    else
        m_optionsToolBar = new ACTION_TOOLBAR( this, ID_OPT_TOOLBAR,
                                               wxDefaultPosition, wxDefaultSize,
                                               KICAD_AUI_TB_STYLE | wxAUI_TB_VERTICAL );

    m_optionsToolBar->AddTool( ID_TB_OPTIONS_DRC_OFF, wxEmptyString, KiScaledBitmap( drc_off_xpm, this ),
                               _( "Enable design rule checking" ), wxITEM_CHECK );
    m_optionsToolBar->Add( ACTIONS::toggleGrid,              ACTION_TOOLBAR::TOGGLE );

    m_optionsToolBar->Add( PCB_ACTIONS::togglePolarCoords,  ACTION_TOOLBAR::TOGGLE );
    m_optionsToolBar->Add( ACTIONS::imperialUnits,           ACTION_TOOLBAR::TOGGLE );
    m_optionsToolBar->Add( ACTIONS::metricUnits,             ACTION_TOOLBAR::TOGGLE );
    m_optionsToolBar->Add( ACTIONS::toggleCursorStyle,       ACTION_TOOLBAR::TOGGLE );

    KiScaledSeparator( m_optionsToolBar, this );
    m_optionsToolBar->Add( PCB_ACTIONS::showRatsnest,        ACTION_TOOLBAR::TOGGLE );
    m_optionsToolBar->Add( PCB_ACTIONS::ratsnestLineMode,    ACTION_TOOLBAR::TOGGLE );

    KiScaledSeparator( m_optionsToolBar, this );
    m_optionsToolBar->Add( PCB_ACTIONS::zoneDisplayEnable,   ACTION_TOOLBAR::TOGGLE );
    m_optionsToolBar->Add( PCB_ACTIONS::zoneDisplayDisable,  ACTION_TOOLBAR::TOGGLE );
    m_optionsToolBar->Add( PCB_ACTIONS::zoneDisplayOutlines, ACTION_TOOLBAR::TOGGLE );

    KiScaledSeparator( m_optionsToolBar, this );
    m_optionsToolBar->Add( PCB_ACTIONS::padDisplayMode,      ACTION_TOOLBAR::TOGGLE );
    m_optionsToolBar->Add( PCB_ACTIONS::viaDisplayMode,      ACTION_TOOLBAR::TOGGLE );
    m_optionsToolBar->Add( PCB_ACTIONS::trackDisplayMode,    ACTION_TOOLBAR::TOGGLE );
    m_optionsToolBar->Add( PCB_ACTIONS::highContrastMode,    ACTION_TOOLBAR::TOGGLE );

    // Tools to show/hide toolbars:
    KiScaledSeparator( m_optionsToolBar, this );
    m_optionsToolBar->AddTool( ID_TB_OPTIONS_SHOW_MANAGE_LAYERS_VERTICAL_TOOLBAR,
                               wxEmptyString,
                               KiScaledBitmap( layers_manager_xpm, this ),
                               HELP_SHOW_HIDE_LAYERMANAGER,
                               wxITEM_CHECK );
    m_optionsToolBar->AddTool( ID_TB_OPTIONS_SHOW_EXTRA_VERTICAL_TOOLBAR_MICROWAVE,
                               wxEmptyString,
                               KiScaledBitmap( mw_toolbar_xpm, this ),
                               HELP_SHOW_HIDE_MICROWAVE_TOOLS,
                               wxITEM_CHECK );


    KiScaledSeparator( m_optionsToolBar, this );
    m_optionsToolBar->Realize();
}


void PCB_EDIT_FRAME::ReCreateVToolbar()
{
    wxWindowUpdateLocker dummy( this );

    if( m_drawToolBar )
        m_drawToolBar->Clear();
    else
        m_drawToolBar = new ACTION_TOOLBAR( this, ID_V_TOOLBAR, wxDefaultPosition, wxDefaultSize,
                                            KICAD_AUI_TB_STYLE | wxAUI_TB_VERTICAL );

    m_drawToolBar->Add( PCB_ACTIONS::selectionTool,        ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::highlightNetTool,     ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::localRatsnestTool,    ACTION_TOOLBAR::TOGGLE );

    KiScaledSeparator( m_drawToolBar, this );
    m_drawToolBar->Add( PCB_ACTIONS::placeModule,          ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::routerActivateSingle, ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::drawVia,              ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::drawZone,             ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::drawZoneKeepout,      ACTION_TOOLBAR::TOGGLE );

    KiScaledSeparator( m_drawToolBar, this );
    m_drawToolBar->Add( PCB_ACTIONS::drawLine,             ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::drawCircle,           ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::drawArc,              ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::drawPolygon,          ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::placeText,            ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::drawDimension,        ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::placeTarget,          ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::deleteTool,           ACTION_TOOLBAR::TOGGLE );

    KiScaledSeparator( m_drawToolBar, this );
    m_drawToolBar->Add( PCB_ACTIONS::drillOrigin,          ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::gridSetOrigin,        ACTION_TOOLBAR::TOGGLE );
    m_drawToolBar->Add( PCB_ACTIONS::measureTool,          ACTION_TOOLBAR::TOGGLE );

    m_drawToolBar->Realize();
}


/* Create the auxiliary vertical right toolbar, showing tools for microwave applications
 */
void PCB_EDIT_FRAME::ReCreateMicrowaveVToolbar()
{
    wxWindowUpdateLocker dummy(this);

    if( m_microWaveToolBar )
        m_microWaveToolBar->Clear();
    else
        m_microWaveToolBar = new wxAuiToolBar( this, ID_MICROWAVE_V_TOOLBAR, wxDefaultPosition,
                                               wxDefaultSize,
                                               KICAD_AUI_TB_STYLE | wxAUI_TB_VERTICAL );

    // Set up toolbar
    m_microWaveToolBar->AddTool( ID_PCB_MUWAVE_TOOL_SELF_CMD, wxEmptyString,
                                 KiScaledBitmap( mw_add_line_xpm, this ),
                                 _( "Create line of specified length for microwave applications" ),
                                 wxITEM_CHECK );

    m_microWaveToolBar->AddTool( ID_PCB_MUWAVE_TOOL_GAP_CMD, wxEmptyString,
                                 KiScaledBitmap( mw_add_gap_xpm, this ),
                                 _( "Create gap of specified length for microwave applications" ),
                                 wxITEM_CHECK );

    KiScaledSeparator( m_microWaveToolBar, this );

    m_microWaveToolBar->AddTool( ID_PCB_MUWAVE_TOOL_STUB_CMD, wxEmptyString,
                                 KiScaledBitmap( mw_add_stub_xpm, this ),
                                 _( "Create stub of specified length for microwave applications" ),
                                 wxITEM_CHECK );

    m_microWaveToolBar->AddTool( ID_PCB_MUWAVE_TOOL_STUB_ARC_CMD, wxEmptyString,
                                 KiScaledBitmap( mw_add_stub_arc_xpm, this ),
                                 _( "Create stub (arc) of specified length for microwave applications" ),
                                 wxITEM_CHECK );

    m_microWaveToolBar->AddTool( ID_PCB_MUWAVE_TOOL_FUNCTION_SHAPE_CMD, wxEmptyString,
                                 KiScaledBitmap( mw_add_shape_xpm, this ),
                                 _( "Create a polynomial shape for microwave applications" ),
                                 wxITEM_CHECK );

    m_microWaveToolBar->Realize();
}


void PCB_EDIT_FRAME::ReCreateAuxiliaryToolbar()
{
    wxWindowUpdateLocker dummy( this );

    if( m_auxiliaryToolBar )
    {
        UpdateTrackWidthSelectBox( m_SelTrackWidthBox );
        UpdateViaSizeSelectBox( m_SelViaSizeBox );

        // combobox sizes can have changed: apply new best sizes
        wxAuiToolBarItem* item = m_auxiliaryToolBar->FindTool( ID_AUX_TOOLBAR_PCB_TRACK_WIDTH );
        item->SetMinSize( m_SelTrackWidthBox->GetBestSize() );
        item = m_auxiliaryToolBar->FindTool( ID_AUX_TOOLBAR_PCB_VIA_SIZE );
        item->SetMinSize( m_SelViaSizeBox->GetBestSize() );

        m_auxiliaryToolBar->Realize();
        m_auimgr.Update();
        return;
    }

    m_auxiliaryToolBar = new ACTION_TOOLBAR( this, ID_AUX_TOOLBAR,
                                             wxDefaultPosition, wxDefaultSize,
                                             KICAD_AUI_TB_STYLE | wxAUI_TB_HORZ_LAYOUT );

    /* Set up toolbar items */

    // Creates box to display and choose tracks widths:
    if( m_SelTrackWidthBox == nullptr )
        m_SelTrackWidthBox = new wxChoice( m_auxiliaryToolBar,
                                       ID_AUX_TOOLBAR_PCB_TRACK_WIDTH,
                                       wxDefaultPosition, wxDefaultSize,
                                       0, NULL );

    UpdateTrackWidthSelectBox( m_SelTrackWidthBox );
    m_auxiliaryToolBar->AddControl( m_SelTrackWidthBox );

    // Creates box to display and choose vias diameters:

    if( m_SelViaSizeBox == nullptr )
        m_SelViaSizeBox = new wxChoice( m_auxiliaryToolBar,
                                    ID_AUX_TOOLBAR_PCB_VIA_SIZE,
                                    wxDefaultPosition, wxDefaultSize,
                                    0, NULL );

    UpdateViaSizeSelectBox( m_SelViaSizeBox );
    m_auxiliaryToolBar->AddControl( m_SelViaSizeBox );
    KiScaledSeparator( m_auxiliaryToolBar, this );

    // Creates box to display and choose strategy to handle tracks an vias sizes:
    m_auxiliaryToolBar->AddTool( ID_AUX_TOOLBAR_PCB_SELECT_AUTO_WIDTH,
                                 wxEmptyString,
                                 KiScaledBitmap( auto_track_width_xpm, this ),
                                 _( "Auto track width: when starting on an existing track "
                                    "use its width\notherwise, use current width setting" ),
                                 wxITEM_CHECK );

    // Add the box to display and select the current grid size:
    KiScaledSeparator( m_auxiliaryToolBar, this );

    if( m_gridSelectBox == nullptr )
        m_gridSelectBox = new wxChoice( m_auxiliaryToolBar,
                                    ID_ON_GRID_SELECT,
                                    wxDefaultPosition, wxDefaultSize,
                                    0, NULL );

    UpdateGridSelectBox();

    m_auxiliaryToolBar->AddControl( m_gridSelectBox );

    //  Add the box to display and select the current Zoom
    KiScaledSeparator( m_auxiliaryToolBar, this );

    if( m_zoomSelectBox == nullptr )
        m_zoomSelectBox = new wxChoice( m_auxiliaryToolBar,
                                    ID_ON_ZOOM_SELECT,
                                    wxDefaultPosition, wxDefaultSize,
                                    0, NULL );

    updateZoomSelectBox();
    m_auxiliaryToolBar->AddControl( m_zoomSelectBox );

    // after adding the buttons to the toolbar, must call Realize()
    m_auxiliaryToolBar->Realize();
}


void PCB_EDIT_FRAME::UpdateTrackWidthSelectBox( wxChoice* aTrackWidthSelectBox, const bool aEdit )
{
    if( aTrackWidthSelectBox == NULL )
        return;

    wxString msg;
    bool mmFirst = GetUserUnits() != INCHES;

    aTrackWidthSelectBox->Clear();

    for( unsigned ii = 0; ii < GetDesignSettings().m_TrackWidthList.size(); ii++ )
    {
        int size = GetDesignSettings().m_TrackWidthList[ii];

        double valueMils = To_User_Unit( INCHES, size ) * 1000;
        double value_mm = To_User_Unit( MILLIMETRES, size );

        if( mmFirst )
            msg.Printf( _( "Track: %.3f mm (%.2f mils)" ),
                        value_mm, valueMils );
        else
            msg.Printf( _( "Track: %.2f mils (%.3f mm)" ),
                        valueMils, value_mm );

        // Mark the netclass track width value (the first in list)
        if( ii == 0 )
            msg << wxT( " *" );

        aTrackWidthSelectBox->Append( msg );
    }

    if( aEdit )
    {
        aTrackWidthSelectBox->Append( wxT( "---" ) );
        aTrackWidthSelectBox->Append( _( "Edit pre-defined sizes..." ) );
    }

    if( GetDesignSettings().GetTrackWidthIndex() >= GetDesignSettings().m_TrackWidthList.size() )
        GetDesignSettings().SetTrackWidthIndex( 0 );

    aTrackWidthSelectBox->SetSelection( GetDesignSettings().GetTrackWidthIndex() );
}


void PCB_EDIT_FRAME::UpdateViaSizeSelectBox( wxChoice* aViaSizeSelectBox, const bool aEdit )
{
    if( aViaSizeSelectBox == NULL )
        return;

    aViaSizeSelectBox->Clear();

    bool mmFirst = GetUserUnits() != INCHES;

    for( unsigned ii = 0; ii < GetDesignSettings().m_ViasDimensionsList.size(); ii++ )
    {
        VIA_DIMENSION viaDimension = GetDesignSettings().m_ViasDimensionsList[ii];
        wxString      msg, mmStr, milsStr;

        double diam = To_User_Unit( MILLIMETRES, viaDimension.m_Diameter );
        double hole = To_User_Unit( MILLIMETRES, viaDimension.m_Drill );

        if( hole > 0 )
            mmStr.Printf( _( "%.2f / %.2f mm" ), diam, hole );
        else
            mmStr.Printf( _( "%.2f mm" ), diam );

        diam = To_User_Unit( INCHES, viaDimension.m_Diameter ) * 1000;
        hole = To_User_Unit( INCHES, viaDimension.m_Drill ) * 1000;

        if( hole > 0 )
            milsStr.Printf( _( "%.1f / %.1f mils" ), diam, hole );
        else
            milsStr.Printf( _( "%.1f mils" ), diam );

        msg.Printf( _( "Via: %s (%s)" ), mmFirst ? mmStr : milsStr, mmFirst ? milsStr : mmStr );

        // Mark the netclass via size value (the first in list)
        if( ii == 0 )
            msg << wxT( " *" );

        aViaSizeSelectBox->Append( msg );
    }

    if( aEdit )
    {
        aViaSizeSelectBox->Append( wxT( "---" ) );
        aViaSizeSelectBox->Append( _( "Edit pre-defined sizes..." ) );
    }

    if( GetDesignSettings().GetViaSizeIndex() >= GetDesignSettings().m_ViasDimensionsList.size() )
        GetDesignSettings().SetViaSizeIndex( 0 );

    aViaSizeSelectBox->SetSelection( GetDesignSettings().GetViaSizeIndex() );
}


void PCB_EDIT_FRAME::ReCreateLayerBox( bool aForceResizeToolbar )
{
    if( m_SelLayerBox == NULL || m_mainToolBar == NULL )
        return;

    m_SelLayerBox->SetToolTip( _( "+/- to switch" ) );
    m_SelLayerBox->m_hotkeys = g_Board_Editor_Hotkeys_Descr;
    m_SelLayerBox->Resync();

    if( aForceResizeToolbar )
    {
        // the layer box can have its size changed
        // Update the aui manager, to take in account the new size
        m_auimgr.Update();
    }
}


void PCB_EDIT_FRAME::OnSelectOptionToolbar( wxCommandEvent& event )
{
    int id = event.GetId();
    bool state = event.IsChecked();

    switch( id )
    {
    case ID_TB_OPTIONS_DRC_OFF:
        Settings().m_legacyDrcOn = !state;

        if( GetToolId() == ID_TRACK_BUTT )
        {
            if( Settings().m_legacyDrcOn )
                m_canvas->SetCursor( wxCURSOR_PENCIL );
            else
                m_canvas->SetCursor( wxCURSOR_QUESTION_ARROW );
        }
        break;

    case ID_TB_OPTIONS_SHOW_EXTRA_VERTICAL_TOOLBAR_MICROWAVE:
        m_show_microwave_tools = state;
        m_auimgr.GetPane( "MicrowaveToolbar" ).Show( m_show_microwave_tools );
        m_auimgr.Update();
        break;

    case ID_TB_OPTIONS_SHOW_MANAGE_LAYERS_VERTICAL_TOOLBAR:
        // show auxiliary Vertical layers and visibility manager toolbar
        m_show_layer_manager_tools = state;
        m_auimgr.GetPane( "LayersManager" ).Show( m_show_layer_manager_tools );
        m_auimgr.Update();
        break;

    default:
        DisplayErrorMessage( this, "Invalid toolbar option",
                       "PCB_EDIT_FRAME::OnSelectOptionToolbar error \n (event not handled!)" );
        break;
    }
}


void PCB_EDIT_FRAME::OnUpdateLayerPair( wxUpdateUIEvent& aEvent )
{
    PrepareLayerIndicator();
}


void PCB_EDIT_FRAME::OnUpdateSelectTrackWidth( wxUpdateUIEvent& aEvent )
{
    if( aEvent.GetId() == ID_AUX_TOOLBAR_PCB_TRACK_WIDTH )
    {
        if( m_SelTrackWidthBox->GetSelection() != (int) GetDesignSettings().GetTrackWidthIndex() )
            m_SelTrackWidthBox->SetSelection( GetDesignSettings().GetTrackWidthIndex() );
    }
}


void PCB_EDIT_FRAME::OnUpdateSelectViaSize( wxUpdateUIEvent& aEvent )
{
    if( aEvent.GetId() == ID_AUX_TOOLBAR_PCB_VIA_SIZE )
    {
        if( m_SelViaSizeBox->GetSelection() != (int) GetDesignSettings().GetViaSizeIndex() )
            m_SelViaSizeBox->SetSelection( GetDesignSettings().GetViaSizeIndex() );
    }
}


void PCB_EDIT_FRAME::OnUpdateLayerSelectBox( wxUpdateUIEvent& aEvent )
{
    m_SelLayerBox->SetLayerSelection( GetActiveLayer() );
}


#if defined( KICAD_SCRIPTING_WXPYTHON )

// Used only when the DKICAD_SCRIPTING_WXPYTHON option is on
void PCB_EDIT_FRAME::OnUpdateScriptingConsoleState( wxUpdateUIEvent& aEvent )
{
    if( aEvent.GetEventObject() != m_mainToolBar )
        return;

    wxMiniFrame* pythonPanelFrame = (wxMiniFrame *) findPythonConsole();
    bool pythonPanelShown = pythonPanelFrame ? pythonPanelFrame->IsShown() : false;
    aEvent.Check( pythonPanelShown );
}

#endif


void PCB_EDIT_FRAME::OnUpdateDrcEnable( wxUpdateUIEvent& aEvent )
{
    bool state = !Settings().m_legacyDrcOn;
    aEvent.Check( state );
    m_optionsToolBar->SetToolShortHelp( ID_TB_OPTIONS_DRC_OFF,
                                        Settings().m_legacyDrcOn ?
                                        _( "Disable design rule checking while routing/editing tracks using Legacy Toolset.\nUse Route > Interactive Router Settings... for Modern Toolset." ) :
                                        _( "Enable design rule checking while routing/editing tracks using Legacy Toolset.\nUse Route > Interactive Router Settings... for Modern Toolset." ) );
}

bool PCB_EDIT_FRAME::LayerManagerShown()
{
    return m_auimgr.GetPane( "LayersManager" ).IsShown();
}

bool PCB_EDIT_FRAME::MicrowaveToolbarShown()
{
    return m_auimgr.GetPane( "MicrowaveToolbar" ).IsShown();
}


void PCB_EDIT_FRAME::OnUpdateSave( wxUpdateUIEvent& aEvent )
{
    aEvent.Enable( GetScreen()->IsModify() );
}


void PCB_EDIT_FRAME::OnUpdateVerticalToolbar( wxUpdateUIEvent& aEvent )
{
    if( aEvent.GetEventObject() == m_drawToolBar || aEvent.GetEventObject() == m_mainToolBar )
        aEvent.Check( GetToolId() == aEvent.GetId() );
}

void PCB_EDIT_FRAME::OnUpdateMuWaveToolbar( wxUpdateUIEvent& aEvent )
{
    if( aEvent.GetEventObject() == m_microWaveToolBar )
        aEvent.Check( GetToolId() == aEvent.GetId() );
}


void PCB_EDIT_FRAME::SyncMenusAndToolbars()
{
    PCB_DISPLAY_OPTIONS*        opts = (PCB_DISPLAY_OPTIONS*) GetDisplayOptions();
    KIGFX::GAL_DISPLAY_OPTIONS& galOpts = GetGalDisplayOptions();
    int                         zoneMode = opts->m_DisplayZonesMode;

    m_mainToolBar->Toggle( ACTIONS::undo, GetScreen() && GetScreen()->GetUndoCommandCount() > 0 );
    m_mainToolBar->Toggle( ACTIONS::redo, GetScreen() && GetScreen()->GetRedoCommandCount() > 0 );
    m_mainToolBar->Toggle( ACTIONS::zoomTool, GetToolId() == ID_ZOOM_SELECTION );
    m_mainToolBar->Refresh();

    m_optionsToolBar->Toggle( ACTIONS::toggleGrid,            IsGridVisible() );
    m_optionsToolBar->Toggle( ACTIONS::metricUnits,           GetUserUnits() != INCHES );
    m_optionsToolBar->Toggle( ACTIONS::imperialUnits,         GetUserUnits() == INCHES );
    m_optionsToolBar->Toggle( ACTIONS::togglePolarCoords,     GetShowPolarCoords() );
    m_optionsToolBar->Toggle( ACTIONS::toggleCursorStyle,     !galOpts.m_fullscreenCursor );
    m_optionsToolBar->Toggle( PCB_ACTIONS::showRatsnest,      GetBoard()->IsElementVisible( LAYER_RATSNEST ) );
    m_optionsToolBar->Toggle( PCB_ACTIONS::ratsnestLineMode,  opts->m_DisplayRatsnestLinesCurved );

    m_optionsToolBar->Toggle( PCB_ACTIONS::zoneDisplayEnable,   zoneMode == 0 );
    m_optionsToolBar->Toggle( PCB_ACTIONS::zoneDisplayDisable,  zoneMode == 1 );
    m_optionsToolBar->Toggle( PCB_ACTIONS::zoneDisplayOutlines, zoneMode == 2 );
    m_optionsToolBar->Toggle( PCB_ACTIONS::trackDisplayMode,    !opts->m_DisplayPcbTrackFill );
    m_optionsToolBar->Toggle( PCB_ACTIONS::viaDisplayMode,      !opts->m_DisplayViaFill );
    m_optionsToolBar->Toggle( PCB_ACTIONS::padDisplayMode,      !opts->m_DisplayPadFill );
    m_optionsToolBar->Toggle( PCB_ACTIONS::highContrastMode,    opts->m_ContrastModeDisplay );
    m_optionsToolBar->Refresh();

    m_drawToolBar->Toggle( PCB_ACTIONS::selectionTool,    GetToolId() == ID_NO_TOOL_SELECTED );
    m_drawToolBar->Toggle( PCB_ACTIONS::highlightNetTool, GetToolId() == ID_PCB_HIGHLIGHT_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::localRatsnestTool,GetToolId() == ID_LOCAL_RATSNEST_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::placeModule,      GetToolId() == ID_PCB_MODULE_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::routerActivateSingle, GetToolId() == ID_TRACK_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::drawVia,          GetToolId() == ID_PCB_DRAW_VIA_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::drawZone,         GetToolId() == ID_PCB_ZONES_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::drawZoneKeepout,  GetToolId() == ID_PCB_KEEPOUT_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::drawLine,         GetToolId() == ID_PCB_ADD_LINE_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::drawCircle,       GetToolId() == ID_PCB_CIRCLE_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::drawArc,          GetToolId() == ID_PCB_ARC_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::drawPolygon,      GetToolId() == ID_PCB_ADD_POLYGON_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::placeText,        GetToolId() == ID_PCB_ADD_TEXT_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::drawDimension,    GetToolId() == ID_PCB_DIMENSION_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::placeTarget,      GetToolId() == ID_PCB_TARGET_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::deleteTool,       GetToolId() == ID_PCB_DELETE_ITEM_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::drillOrigin,      GetToolId() == ID_PCB_PLACE_OFFSET_COORD_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::gridSetOrigin,    GetToolId() == ID_PCB_PLACE_GRID_COORD_BUTT );
    m_drawToolBar->Toggle( PCB_ACTIONS::measureTool,      GetToolId() == ID_PCB_MEASUREMENT_TOOL );
    m_drawToolBar->Refresh();
}
