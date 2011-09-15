/*****************
*   track.cpp
*****************/

#include "fctsys.h"

#include "common.h"
#include "pcbnew.h"

#include "protos.h"

/* Functions to recognize a track.
 * A track is a list of connected segments (or/and vias) from a starting to an ending point
 * starting and ending points are a pad or a point with more than 2 segments connected
 * (and obviously a dangling segment end)
 */

typedef std::vector<TRACK*> TRACK_PTRS; // buffer of item candidates when
                                        // search for items on the same track


/* Local functions */
static void ChainMarkedSegments( BOARD* Pcb, wxPoint ref_pos, int masklayer, TRACK_PTRS* aList );


TRACK* MarkTrace( BOARD* aPcb,
                  TRACK* aStartSegm,
                  int*   aSegmCount,
                  int*   aTrackLen,
                  int*   aLengthDie,
                  bool   aReorder )
{
    int        NbSegmBusy;

    TRACK_PTRS trackList;

    if( aSegmCount )
        *aSegmCount = 0;

    if( aTrackLen )
        *aTrackLen = 0;

    if( aStartSegm == NULL )
        return NULL;

    // Ensure the flag BUSY of all tracks of the board is cleared
    // because we use it to mark segments of the track
    for( TRACK* track = aPcb->m_Track; track; track = track->Next() )
        track->SetState( BUSY, OFF );

    /* Set flags of the initial track segment */
    aStartSegm->SetState( BUSY, ON );
    int layerMask = aStartSegm->ReturnMaskLayer();

    trackList.push_back( aStartSegm );

    /* Examine the initial track segment : if it is really a segment, this is
     * easy.
     *  If it is a via, one must search for connected segments.
     *  If <=2, this via connect 2 segments (or is connected to only one
     *  segment) and this via and these 2 segments are a part of a track.
     *  If > 2 only this via is flagged (the track has only this via)
     */
    if( aStartSegm->Type() == TYPE_VIA )
    {
        TRACK* Segm1, * Segm2 = NULL, * Segm3 = NULL;
        Segm1 = GetTrace( aPcb->m_Track, NULL, aStartSegm->m_Start, layerMask );

        if( Segm1 )
        {
            Segm2 = GetTrace( Segm1->Next(), NULL, aStartSegm->m_Start, layerMask );
        }

        if( Segm2 )
        {
            Segm3 = GetTrace( Segm2->Next(), NULL, aStartSegm->m_Start, layerMask );
        }

        if( Segm3 ) // More than 2 segments are connected to this via. the track" is only this via
        {
            if( aSegmCount )
                *aSegmCount = 1;

            return aStartSegm;
        }

        if( Segm1 ) // search for others segments connected to the initial segment start point
        {
            layerMask = Segm1->ReturnMaskLayer();
            ChainMarkedSegments( aPcb, aStartSegm->m_Start, layerMask, &trackList );
        }

        if( Segm2 ) // search for others segments connected to the initial segment end point
        {
            layerMask = Segm2->ReturnMaskLayer();
            ChainMarkedSegments( aPcb, aStartSegm->m_Start, layerMask, &trackList );
        }
    }
    else    // mark the chain using both ends of the initial segment
    {
        ChainMarkedSegments( aPcb, aStartSegm->m_Start, layerMask, &trackList );
        ChainMarkedSegments( aPcb, aStartSegm->m_End, layerMask, &trackList );
    }

    // Now examine selected vias and flag them if they are on the track
    // If a via is connected to only one or 2 segments, it is flagged (is on the track)
    // If a via is connected to more than 2 segments, it is a track end, and it
    // is removed from the list
    // go through the list backwards.
    for( int i = trackList.size() - 1;  i>=0;  --i )
    {
        TRACK* via = trackList[i];

        if( via->Type() != TYPE_VIA )
            continue;

        if( via == aStartSegm )
            continue;

        via->SetState( BUSY, ON );  // Try to flag it. the flag will be cleared later if needed

        layerMask = via->ReturnMaskLayer();

        TRACK* track = GetTrace( aPcb->m_Track, NULL, via->m_Start, layerMask );

        // GetTrace does not consider tracks flagged BUSY.
        // So if no connected track found, this via is on the current track
        // only: keep it
        if( track == NULL )
            continue;

        /* If a track is found, this via connects also others segments of an
         * other track.  This case happens when the vias ends the selected
         * track but must we consider this via is on the selected track, or
         * on an other track.
         * (this is important when selecting a track for deletion: must this
         * via be deleted or not?)
         * We consider here this via on the track if others segment connected
         * to this via remain connected when removing this via.
         * We search for all others segment connected together:
         * if there are on the same layer, the via is on the selected track
         * if there are on different layers, the via is on an other track
         */
        int layer = track->GetLayer();

        while( ( track = GetTrace( track->Next(), NULL, via->m_Start, layerMask ) ) != NULL )
        {
            if( layer != track->GetLayer() )
            {
                // The via connects segments of an other track: it is removed
                // from list because it is member of an other track
                via->SetState( BUSY, OFF );
                break;
            }
        }
    }

    /* Rearrange the track list in order to have flagged segments linked
     * from firstTrack so the NbSegmBusy segments are consecutive segments
     * in list, the first item in the full track list is firstTrack, and
     * the NbSegmBusy-1 next items (NbSegmBusy when including firstTrack)
     * are the flagged segments
     */
    NbSegmBusy = 0;
    TRACK* firstTrack;

    for( firstTrack = aPcb->m_Track; firstTrack; firstTrack = firstTrack->Next() )
    {
        // Search for the first flagged BUSY segments
        if( firstTrack->GetState( BUSY ) )
        {
            NbSegmBusy = 1;
            break;
        }
    }

    if( firstTrack == NULL )
        return NULL;

    double full_len = 0;
    double lenDie = 0;

    if( aReorder )
    {
        DLIST<TRACK>* list = (DLIST<TRACK>*)firstTrack->GetList();
        wxASSERT( list );

        /* Rearrange the chain starting at firstTrack
         * All others flagged items are moved from their position to the end
         * of the flagged list
         */
        TRACK* next;

        for( TRACK* track = firstTrack->Next(); track; track = next )
        {
            next = track->Next();

            if( track->GetState( BUSY ) )   // move it!
            {
                NbSegmBusy++;
                track->UnLink();
                list->Insert( track, firstTrack->Next() );

                if( aTrackLen )
                    full_len += track->GetLength();

                if( aLengthDie ) // Add now length die.
                {
                    // In fact only 2 pads (maximum) will be taken in account:
                    // that are on each end of the track, if any
                    if( track->GetState( BEGIN_ONPAD ) )
                    {
                        D_PAD * pad = (D_PAD *) track->start;
                        lenDie += (double) pad->m_LengthDie;
                    }

                    if( track->GetState( END_ONPAD ) )
                    {
                        D_PAD * pad = (D_PAD *) track->end;
                        lenDie += (double) pad->m_LengthDie;
                    }
                }
            }
        }
    }
    else if( aTrackLen )
    {
        NbSegmBusy = 0;

        for( TRACK* track = firstTrack; track; track = track->Next() )
        {
            if( track->GetState( BUSY ) )
            {
                NbSegmBusy++;
                track->SetState( BUSY, OFF );
                full_len += track->GetLength();

                // Add now length die.
                // In fact only 2 pads (maximum) will be taken in account:
                // that are on each end of the track, if any
                if( track->GetState( BEGIN_ONPAD ) )
                {
                    D_PAD * pad = (D_PAD *) track->start;
                    lenDie += (double) pad->m_LengthDie;
                }

                if( track->GetState( END_ONPAD ) )
                {
                    D_PAD * pad = (D_PAD *) track->end;
                    lenDie += (double) pad->m_LengthDie;
                }
            }
        }
    }

    if( aTrackLen )
        *aTrackLen = wxRound( full_len );

    if( aLengthDie )
        *aLengthDie = wxRound( lenDie );

    if( aSegmCount )
        *aSegmCount = NbSegmBusy;

    return firstTrack;
}


/**
 * Function used by MarkTrace
 *  - Set the BUSY flag of connected segments, the first search point is
 *      ref_pos on layers allowed in masque_layer
 *  - Put segments fount in aList
 *  Vias are put in list but their flags BUSY is not set
 * @param aPcb = the board
 * @param aRef_pos = the reference coordinate of the starting search
 * @param aLayerMask = the allowed layers for segments to search
 *  (1 layer when starting point is on a segment, but more than one when
 *  starting point is on a via)
 * @param aList = the track list to fill with points of segments flagged
 */
static void ChainMarkedSegments( BOARD*      aPcb,
                                 wxPoint     aRef_pos,
                                 int         aLayerMask,
                                 TRACK_PTRS* aList )
{
    TRACK* pt_segm,             // The current segment being analyzed.
    * pt_via,                   // The via identified, eventually destroy

    * SegmentCandidate;         // The end segment to destroy (or NULL = pt_segm
    int NbSegm;

    if( aPcb->m_Track == NULL )
        return;

    /* Set the BUSY flag of all connected segments, first search starting at
     * aRef_pos
     *  Search ends when:
     *     - a pad is found (end of a track)
     *     - a segment end has more than one other segment end connected
     *     - and obviously when no connected item found
     *  Vias are a special case, because we must see others segment connected
     *  on others layers and they change the layer mask. They can be a track
     *  end or not
     * They will be analyzer later, and vias on terminal points of the track
     * will be considered as part of this track if they do not connect segments
     * of an other track together and will be considered as part of an other
     * track if when removing the via, the segments of that other track are
     * disconnected
     */
    for( ; ; )
    {
        if( aPcb->GetPadFast( aRef_pos, aLayerMask ) != NULL )
            return;

        /* Test for a via: a via changes the layer mask and can connect a lot
         * of segments at location aRef_pos. When found, the via is just
         * pushed in list.  Vias will be examined later, when all connected
         * segment are found and push in list.  This is because when a via
         * is found we do not know at this time the number of connected items
         * and we do not know if this via is on the track or finish the track
         */
        pt_via = aPcb->m_Track->GetVia( NULL, aRef_pos, aLayerMask );

        if( pt_via )
        {
            aLayerMask = pt_via->ReturnMaskLayer();

            aList->push_back( pt_via );
        }

        /* Now we search all segments connected to point aRef_pos
         *  if only 1 segment: this segment is candidate
         *  if > 1 segment:
         *      end of track (more than 2 segment connected at this location)
         */
        pt_segm = aPcb->m_Track; SegmentCandidate = NULL;
        NbSegm  = 0;

        while( ( pt_segm = GetTrace( pt_segm, NULL, aRef_pos, aLayerMask ) ) != NULL )
        {
            if( pt_segm->GetState( BUSY ) ) // already found and selected: skip it
            {
                pt_segm = pt_segm->Next();
                continue;
            }

            if( pt_segm == pt_via ) // just previously found: skip it
            {
                pt_segm = pt_segm->Next();
                continue;
            }

            NbSegm++;

            if( NbSegm == 1 ) /* First time we found a connected item: pt_segm is candidate */
            {
                SegmentCandidate = pt_segm;
                pt_segm = pt_segm->Next();
            }
            else /* More than 1 segment connected -> this location is an end of the track */
            {
                return;
            }
        }

        if( SegmentCandidate )      // A candidate is found: flag it an push it in list
        {
            /* Initialize parameters to search items connected to this
             * candidate:
             * we must analyze connections to its other end
             */
            aLayerMask = SegmentCandidate->ReturnMaskLayer();

            if( aRef_pos == SegmentCandidate->m_Start )
            {
                aRef_pos = SegmentCandidate->m_End;
            }
            else
            {
                aRef_pos = SegmentCandidate->m_Start;
            }

            pt_segm = aPcb->m_Track; /* restart list of tracks to analyze */

            /* flag this item an push it in list of selected items */
            aList->push_back( SegmentCandidate );
            SegmentCandidate->SetState( BUSY, ON );
        }
        else
        {
            return;
        }
    }
}


/* Calculate the end points coordinates of a track (a list of connected
 * segments)
 * RefTrack is a segment of the track
 *  return 1 if OK, 0 when a track is a closed loop
 *  and the beginning and the end of the track in *StartTrack and *EndTrack
 *  Modify *StartTrack en *EndTrack  :
 *  (*StartTrack)->m_Start coordinate is the beginning of the track
 *  (*EndTrack)->m_End coordinate is the end of the track
 *  Segments connected must be consecutive in list
 */
int ReturnEndsTrack( TRACK* RefTrack, int NbSegm, TRACK** StartTrack, TRACK** EndTrack )
{
    TRACK* Track, * via, * segm, * TrackListEnd;
    int    NbEnds, layerMask, ii, ok = 0;

    if( NbSegm <= 1 )
    {
        *StartTrack = *EndTrack = RefTrack;
        return 1;
    }

    /* Calculation of the limit analysis. */
    *StartTrack  = *EndTrack = NULL;
    TrackListEnd = Track = RefTrack; ii = 0;

    for( ; ( Track != NULL ) && ( ii < NbSegm ); ii++, Track = Track->Next() )
    {
        TrackListEnd   = Track;
        Track->m_Param = 0;
    }

    /* Calculate the extremes. */
    NbEnds = 0; Track = RefTrack; ii = 0;

    for( ; ( Track != NULL ) && ( ii < NbSegm ); ii++, Track = Track->Next() )
    {
        if( Track->Type() == TYPE_VIA )
            continue;

        layerMask = Track->ReturnMaskLayer();
        via = RefTrack->GetVia( TrackListEnd, Track->m_Start, layerMask );

        if( via )
        {
            layerMask |= via->ReturnMaskLayer();
            via->SetState( BUSY, ON );
        }

        Track->SetState( BUSY, ON );
        segm = GetTrace( RefTrack, TrackListEnd, Track->m_Start, layerMask );
        Track->SetState( BUSY, OFF );

        if( via )
            via->SetState( BUSY, OFF );

        if( segm == NULL )
        {
            switch( NbEnds )
            {
            case 0:
                *StartTrack = Track; NbEnds++;
                break;

            case 1:
                int BeginPad, EndPad;
                *EndTrack = Track;

                /* Swap ox, oy with fx, fy */
                BeginPad = Track->GetState( BEGIN_ONPAD );
                EndPad   = Track->GetState( END_ONPAD );

                Track->SetState( BEGIN_ONPAD | END_ONPAD, OFF );

                if( BeginPad )
                    Track->SetState( END_ONPAD, ON );

                if( EndPad )
                    Track->SetState( BEGIN_ONPAD, ON );

                EXCHG( Track->m_Start, Track->m_End );
                EXCHG( Track->start, Track->end );
                ok = 1;
                return ok;
            }
        }

        layerMask = Track->ReturnMaskLayer();
        via = RefTrack->GetVia( TrackListEnd, Track->m_End, layerMask );

        if( via )
        {
            layerMask |= via->ReturnMaskLayer();
            via->SetState( BUSY, ON );
        }

        Track->SetState( BUSY, ON );
        segm = GetTrace( RefTrack, TrackListEnd, Track->m_End, layerMask );
        Track->SetState( BUSY, OFF );

        if( via )
            via->SetState( BUSY, OFF );

        if( segm == NULL )
        {
            switch( NbEnds )
            {
            case 0:
                int BeginPad, EndPad;
                *StartTrack = Track;
                NbEnds++;

                /* Swap ox, oy with fx, fy */
                BeginPad = Track->GetState( BEGIN_ONPAD );
                EndPad   = Track->GetState( END_ONPAD );

                Track->SetState( BEGIN_ONPAD | END_ONPAD, OFF );

                if( BeginPad )
                    Track->SetState( END_ONPAD, ON );

                if( EndPad )
                    Track->SetState( BEGIN_ONPAD, ON );

                EXCHG( Track->m_Start, Track->m_End );
                EXCHG( Track->start, Track->end );
                break;

            case 1:
                *EndTrack = Track;
                ok = 1;
                return ok;
            }
        }
    }

    return ok;
}
