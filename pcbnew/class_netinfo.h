/**
 * @file class_netinfo.h
 */

/*
 *  Classes to handle info on nets
 */

#ifndef __CLASSES_NETINFO__
#define __CLASSES_NETINFO__


#include <vector>
#include <gr_basic.h>
#include <class_netclass.h>


class wxDC;
class wxPoint;
class LINE_READER;
class EDA_DRAW_PANEL;
class EDA_DRAW_FRAME;
class NETINFO_ITEM;
class D_PAD;
class BOARD;
class BOARD_ITEM;
class MSG_PANEL_ITEM;


/*****************************/
/* flags for a RATSNEST_ITEM */
/*****************************/
#define CH_VISIBLE          1        /* Visible */
#define CH_UNROUTABLE       2        /* Don't use autorouter. */
#define CH_ROUTE_REQ        4        /* Must be routed by the autorouter. */
#define CH_ACTIF            8        /* Not routed. */
#define LOCAL_RATSNEST_ITEM 0x8000   /* Line between two pads of a single module. */


/**
 * Class RATSNEST_ITEM
 * describes a ratsnest line: a straight line connecting 2 pads
 */
class RATSNEST_ITEM
{
private:
    int m_NetCode;      // netcode ( = 1.. n ,  0 is the value used for not connected items)

public:
    int    m_Status;    // State: see previous defines (CH_ ...)
    D_PAD* m_PadStart;  // pointer to the starting pad
    D_PAD* m_PadEnd;    // pointer to ending pad
    int    m_Lenght;    // length of the line (used in some calculations)

    RATSNEST_ITEM();

    /**
     * Function GetNet
     * @return int - the net code.
     */
    int GetNet() const
    {
        return m_NetCode;
    }

    void SetNet( int aNetCode )
    {
        m_NetCode = aNetCode;
    }

    bool IsVisible()
    {
        return (m_Status & CH_VISIBLE) != 0;
    }

    bool IsActive()
    {
        return (m_Status & CH_ACTIF) != 0;
    }

    bool IsLocal()
    {
        return (m_Status & LOCAL_RATSNEST_ITEM) != 0;
    }

    /**
     * Function Draw
     */
    void Draw( EDA_DRAW_PANEL* panel, wxDC* DC, GR_DRAWMODE aDrawMode,
               const wxPoint& offset );
};



/**
 * Class NETINFO
 * is a container class for NETINFO_ITEM elements, which are the nets.  That makes
 * this class a container for the nets.
 */
class NETINFO_LIST
{
    friend class BOARD;

public:
    NETINFO_LIST( BOARD* aParent );
    ~NETINFO_LIST();

    /**
     * Function GetItem
     * @param aNetcode = netcode to identify a given NETINFO_ITEM
     * @return NETINFO_ITEM* - by \a aNetcode, or NULL if not found
     */
    NETINFO_ITEM* GetNetItem( int aNetcode ) const
    {
        if( unsigned( aNetcode ) >= GetNetCount() )     // catches < 0 too
            return NULL;
        return m_NetBuffer[aNetcode];
    }

    /**
     * Function GetNetCount
     * @return the number of nets ( always >= 1 )
     * because the first net is the "not connected" net and always exists
     */
    unsigned GetNetCount() const { return m_NetBuffer.size(); }

    /**
     * Function Append
     * adds \a aNewElement to the end of the list.
     */
    void AppendNet( NETINFO_ITEM* aNewElement );

    /**
     * Function GetPadCount
     * @return the number of pads in board
     */
    unsigned GetPadCount() const  { return m_PadsFullList.size(); }

    /**
     * Function GetPads
     * returns a list of all the pads.  The returned list is not
     * sorted and contains pointers to PADS, but those pointers do not convey
     * ownership of the respective PADs.
     * @return std::vector<D_PAD*>& - a full list of pads
    std::vector<D_PAD*>& GetPads()
    {
        return m_PadsFullList;
    }
     */

    /**
     * Function GetPad
     * @return the pad idx from m_PadsFullList
     */
    D_PAD* GetPad( unsigned aIdx ) const
    {
        if( aIdx < m_PadsFullList.size() )
            return m_PadsFullList[aIdx];
        else
            return NULL;
    }

private:

    /**
     * Function DeleteData
     * deletes the list of nets (and free memory)
     */
    void clear();

    /**
     * Function BuildListOfNets
     * builds or rebuilds the list of NETINFO_ITEMs
     * The list is sorted by names.
     */
    void buildListOfNets();

    /**
     * Function buildPadsFullList
     * creates the pad list, and initializes:
     *   m_Pads (list of pads)
     * set m_Status_Pcb = LISTE_PAD_OK;
     * and clear for all pads in list the m_SubRatsnest member;
     * clear m_Pcb->m_FullRatsnest
     */
    void buildPadsFullList();

    BOARD*                      m_Parent;
    std::vector<NETINFO_ITEM*>  m_NetBuffer;    ///< net list (name, design constraints ..)

    std::vector<D_PAD*>         m_PadsFullList; ///< contains all pads, sorted by pad's netname.
                                                ///< can be used in ratsnest calculations.
};


/**
 * Class NETINFO_ITEM
 * handles the data for a net
 */
class NETINFO_ITEM
{
private:
    int       m_NetCode;        ///< A number equivalent to the net name.
                                ///< Used for fast comparisons in ratsnest and DRC computations.

    wxString  m_Netname;        ///< Full net name like /mysheet/mysubsheet/vout
                                ///< used by Eeschema

    wxString  m_ShortNetname;   // short net name, like vout from
                                // /mysheet/mysubsheet/vout

    wxString  m_NetClassName;   // Net Class name. if void this is equivalent
                                // to "default" (the first
                                // item of the net classes list

    NETCLASS* m_NetClass;

    BOARD_ITEM* m_parent;       ///< The parent board item object the net belongs to.

public:
    int m_NbNodes;                     // Pads count for this net
    int m_NbLink;                      // Ratsnets count for this net
    int m_NbNoconn;                    // Ratsnets remaining to route count
    int m_Flag;                        // used in some calculations. Had no
                                       // special meaning

    std::vector <D_PAD*> m_PadInNetList;    // List of pads connected to this net

    unsigned m_RatsnestStartIdx;       /* Starting point of ratsnests of this
                                        * net (included) in a general buffer of
                                        * ratsnest (a vector<RATSNEST_ITEM*>
                                        * buffer) */

    unsigned m_RatsnestEndIdx;         // Ending point of ratsnests of this net
                                       // (excluded) in this buffer

    NETINFO_ITEM( BOARD_ITEM* aParent, const wxString& aNetName = wxEmptyString, int aNetCode = 0 );
    ~NETINFO_ITEM();

    /**
     * Function SetClass
     * sets \a aNetclass into this NET
     */
    void SetClass( const NETCLASS* aNetClass )
    {
        m_NetClass = (NETCLASS*) aNetClass;

        if( aNetClass )
            m_NetClassName = aNetClass->GetName();
        else
            m_NetClassName = NETCLASS::Default;
    }

    NETCLASS* GetNetClass()
    {
        return m_NetClass;
    }

    /**
     * Function GetClassName
     * returns the class name
     */
    const wxString& GetClassName() const
    {
        return m_NetClassName;
    }

#if 1

    /**
     * Function GetTrackWidth
     * returns the width of tracks used to route this net.
     */
    int GetTrackWidth()
    {
        wxASSERT( m_NetClass );
        return m_NetClass->GetTrackWidth();
    }

    /**
     * Function GetViaSize
     * returns the size of vias used to route this net
     */
    int GetViaSize()
    {
        wxASSERT( m_NetClass );
        return m_NetClass->GetViaDiameter();
    }

    /**
     * Function GetMicroViaSize
     * returns the size of vias used to route this net
     */
    int GetMicroViaSize()
    {
        wxASSERT( m_NetClass );
        return m_NetClass->GetuViaDiameter();
    }

    /**
     * Function GetViaDrillSize
     * returns the size of via drills used to route this net
     */
    int GetViaDrillSize()
    {
        wxASSERT( m_NetClass );
        return m_NetClass->GetViaDrill();
    }

    /**
     * Function GetViaDrillSize
     * returns the size of via drills used to route this net
     */
    int GetMicroViaDrillSize()
    {
        wxASSERT( m_NetClass );
        return m_NetClass->GetuViaDrill();
    }


#if 0

    /**
     * Function GetViaMinSize
     * returns the Minimum value for via sizes (used in DRC)
     */
    int GetViaMinSize()
    {
        wxASSERT( m_NetClass );
        return m_NetClass->GetViaMinSize();
    }

#endif

    /**
     * Function GetClearance
     * returns the clearance when routing near aBoardItem
     */
    int GetClearance( BOARD_ITEM* aBoardItem )
    {
        wxASSERT( m_NetClass );
        return m_NetClass->GetClearance();
    }

#endif

    /**
     * Function Draw
     * @todo we actually could show a NET, simply show all the tracks and
     *       a pads or net name on pad and vias
     */
    void Draw( EDA_DRAW_PANEL* panel, wxDC* DC, GR_DRAWMODE aDrawMode,
               const wxPoint& offset );

    /**
     * Function GetNet
     * @return int - the netcode
     */
    int GetNet() const { return m_NetCode; }

    void SetNet( int aNetCode ) { m_NetCode = aNetCode; }

    int GetNodesCount() const { return m_PadInNetList.size(); }

    /**
     * Function GetNetname
     * @return const wxString * , a pointer to the full netname
     */
    wxString GetNetname() const { return m_Netname; }

    /**
     * Function GetShortNetname
     * @return const wxString * , a pointer to the short netname
     */
    wxString GetShortNetname() const { return m_ShortNetname; }

    /**
     * Function SetNetname
     * @param aNetname : the new netname
     */
    void SetNetname( const wxString& aNetname );

    void GetMsgPanelInfo( std::vector< MSG_PANEL_ITEM >& aList );
};


/***********************************************************/
/* Description of a trace point for monitoring connections */
/***********************************************************/
#define START_ON_PAD   0x10
#define END_ON_PAD     0x20
#define START_ON_TRACK 0x40
#define END_ON_TRACK   0x80


/* Status bit (OR'ed bits) for class BOARD member .m_Status_Pcb */
enum StatusPcbFlags {
    LISTE_PAD_OK = 1,                    /* Pad list is Ok */
    LISTE_RATSNEST_ITEM_OK = 2,          /* General Ratsnest is Ok */
    RATSNEST_ITEM_LOCAL_OK = 4,          /* current MODULE ratsnest is Ok */
    CONNEXION_OK = 8,                    /* List of connections exists. */
    NET_CODES_OK = 0x10,                 /* Bit indicating that Netcode is OK,
                                          * do not change net name.  */
    DO_NOT_SHOW_GENERAL_RASTNEST = 0x20  /* Do not display the general
                                          * ratsnest (used in module moves) */
};


#endif  // __CLASSES_NETINFO__
