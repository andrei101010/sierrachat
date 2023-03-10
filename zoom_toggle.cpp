#include "sierrachart.h"

SCDLLName("Frozen Tundra - Zoom Toggle")

/*
    Written by Frozen Tundra
*/

SCSFExport scsf_ZoomToggle(SCStudyInterfaceRef sc)
{

    // inputs
    SCInputRef i_Enabled = sc.Input[0];
    SCInputRef i_NumBarsWhenZoomed = sc.Input[1];
    SCInputRef i_KeyCodeForZoom = sc.Input[2];
    SCInputRef i_NumTicksYAxis = sc.Input[3];

    // logging obj
    SCString msg;

    // Configuration
    if (sc.SetDefaults)
    {
        sc.GraphShortName = "Zoom";
        sc.GraphRegion = 0;

        i_Enabled.Name = "Enabled?";
        i_Enabled.SetYesNo(1);

        i_NumBarsWhenZoomed.Name = "Number of bars to see when zoomed?";
        i_NumBarsWhenZoomed.SetInt(3);

        i_KeyCodeForZoom.Name = "ASCII Key Code for zoom? (ex: [SPACE] is 32, [.] is 46, [TAB] is 9)";
        i_KeyCodeForZoom.SetInt(32);
        i_KeyCodeForZoom.SetIntLimits(0, 127);

        i_NumTicksYAxis.Name = "Number of ticks to zoom into on Y-Axis (0=all)";
        i_NumTicksYAxis.SetInt(0);

        // allow study to receive input events from our keyboard
        sc.ReceiveCharacterEvents = 1;

        return;
    }

    // dont do anything if disabled
    if (i_Enabled.GetInt() == 0) return;

    // fetch initial settings from inputs
    int NumBarsWhenZoomed = i_NumBarsWhenZoomed.GetInt();
    int KeyCodeForZoom = i_KeyCodeForZoom.GetInt();
    int NumTicksYAxis = i_NumTicksYAxis.GetInt();

    // persist these values between ticks
    int &PrevChartBarSpacing = sc.GetPersistentInt(0);
    int &PrevNumBarsWhenZoomed = sc.GetPersistentInt(1);
    int &NumFillSpaceBars = sc.GetPersistentInt(2);
    int &IsZoomed = sc.GetPersistentInt(3);

    // grab the chart's current bar spacing
    int ChartBarSpacing = sc.ChartBarSpacing;

    // grab the chart's current fill space number of bars
    if (!NumFillSpaceBars) {
        NumFillSpaceBars = sc.NumFillSpaceBars;
    }

    // calculate the number of visible bars on the chart at the moment
    int NumBarsVisible = sc.IndexOfLastVisibleBar - sc.IndexOfFirstVisibleBar + 1;

    // init if not yet set for initial state
    if (!PrevChartBarSpacing) {
        PrevChartBarSpacing = (NumBarsVisible * ChartBarSpacing) / NumBarsWhenZoomed;
    }

    // listen for key presses in the chart
    // https://www.w3schools.com/charsets/ref_html_ascii.asp
    // example: space bar is ascii code 32;
    int CharCode = sc.CharacterEventCode;

    // key press matches what study is configured for
    if (CharCode == KeyCodeForZoom) {

        // if setting for num bars to display when zoomed in was updated, update our calculations
        if (NumBarsWhenZoomed != PrevNumBarsWhenZoomed) {
            PrevChartBarSpacing = (NumBarsVisible * ChartBarSpacing) / NumBarsWhenZoomed;
            PrevNumBarsWhenZoomed = NumBarsWhenZoomed;
        }

        // toggle the zoom
        sc.ChartBarSpacing = PrevChartBarSpacing;

        // update our prev bar spacing for next time
        PrevChartBarSpacing = ChartBarSpacing;

        if (IsZoomed) {
            // zoom back out
            // put back the wide visible view of bars
            sc.NumFillSpaceBars = NumFillSpaceBars;
            // turn on lock fill space again if we had it on before
            if (NumFillSpaceBars > 0) {
                sc.PreserveFillSpace = 1;
            }

            // un-zoom y-axis if setting was set
            if (NumTicksYAxis > 0) {
                sc.BaseGraphScaleConstRange = 0;
                sc.BaseGraphScaleRangeType = SCALE_AUTO;
            }
        }
        else if (!IsZoomed) {
            // zoom in
            // store our locked fill space num bars
            NumFillSpaceBars = sc.NumFillSpaceBars;
            sc.PreserveFillSpace = 0;
            sc.NumFillSpaceBars = 0;

            // zoom y-axis if setting was set
            if (NumTicksYAxis > 0) {
                sc.BaseGraphScaleRangeType = SCALE_CONSTRANGECENTER;                
                sc.BaseGraphScaleConstRange = (float)NumTicksYAxis * sc.TickSize;

            }

        }

        // toggle our zoom status
        IsZoomed = !IsZoomed;

    }
}
