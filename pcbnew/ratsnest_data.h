/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

/**
 * @file ratsnest_data.h
 * @brief Class that computes missing connections on a PCB.
 */

#ifndef RATSNEST_DATA_H
#define RATSNEST_DATA_H

#include <ttl/halfedge/hetriang.h>
#include <ttl/halfedge/hetraits.h>

#include <math/box2.h>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

class BOARD;
class BOARD_ITEM;
class BOARD_CONNECTED_ITEM;
class MODULE;
class D_PAD;
class SEGVIA;
class TRACK;
class ZONE_CONTAINER;
class CPolyPt;

// Preserve KiCad coding style policy
typedef hed::Node RN_NODE;
typedef hed::NodePtr RN_NODE_PTR;
typedef hed::Edge RN_EDGE;
typedef hed::EdgePtr RN_EDGE_PTR;
typedef hed::EdgeMST RN_EDGE_MST;
typedef boost::shared_ptr<hed::EdgeMST> RN_EDGE_MST_PTR;
typedef hed::Triangulation TRIANGULATOR;

///> General interface for filtering out nodes in search functions.
struct RN_NODE_FILTER : public std::unary_function<const RN_NODE_PTR&, bool>
{
    virtual ~RN_NODE_FILTER() {}

    virtual bool operator()( const RN_NODE_PTR& aNode ) const
    {
        return true;        // By default everything passes
    }
};

///> Filters out nodes that have the flag set.
struct WITHOUT_FLAG : public RN_NODE_FILTER
{
    bool operator()( const RN_NODE_PTR& aNode ) const
    {
        return !aNode->GetFlag();
    }
};

///> Functor comparing if two nodes are equal by their coordinates. It is required to make set of
///> shared pointers work properly.
struct RN_NODE_COMPARE : std::binary_function<RN_NODE_PTR, RN_NODE_PTR, bool>
{
    bool operator()( const RN_NODE_PTR& aNode1, const RN_NODE_PTR& aNode2 ) const
    {
        return ( aNode1->GetX() == aNode2->GetX() && aNode1->GetY() == aNode2->GetY() );
    }
};

///> Functor calculating hash for a given node. It is required to make set of shared pointers
///> work properly.
struct RN_NODE_HASH : std::unary_function<RN_NODE_PTR, std::size_t>
{
    std::size_t operator()( const RN_NODE_PTR& aNode ) const
    {
        std::size_t hash = 2166136261u;

        hash ^= aNode->GetX();
        hash *= 16777619;
        hash ^= aNode->GetY();

        return hash;
    }
};


/**
 * Class RN_LINKS
 * Manages data describing nodes and connections for a given net.
 */
class RN_LINKS
{
public:
    // Helper typedefs
    typedef boost::unordered_set<RN_NODE_PTR, RN_NODE_HASH, RN_NODE_COMPARE> RN_NODE_SET;
    typedef std::list<RN_EDGE_PTR> RN_EDGE_LIST;

    /**
     * Function AddNode()
     * Adds a node with given coordinates and returns pointer to the newly added node. If the node
     * existed before, only appropriate pointer is returned.
     * @param aX is the x coordinate of a node.
     * @param aY is the y coordinate of a node.
     * @return Pointer to the node with given coordinates.
     */
    const RN_NODE_PTR& AddNode( int aX, int aY );

    /**
     * Function RemoveNode()
     * Removes a node described by a given node pointer.
     * @param aNode is a pointer to node to be removed.
     */
    void RemoveNode( const RN_NODE_PTR& aNode );

    /**
     * Function GetNodes()
     * Returns the set of currently used nodes.
     * @return The set of currently used nodes.
     */
    const RN_NODE_SET& GetNodes() const
    {
        return m_nodes;
    }

    /**
     * Function AddConnection()
     * Adds a connection between two nodes and of given distance. Edges with distance equal 0 are
     * considered to be existing connections. Distance different than 0 means that the connection
     * is missing.
     * @param aNode1 is the origin node of a new connection.
     * @param aNode2 is the end node of a new connection.
     * @param aDistance is the distance of the connection (0 means that nodes are actually
     * connected, >0 means a missing connection).
     */
    const RN_EDGE_PTR& AddConnection( const RN_NODE_PTR& aNode1, const RN_NODE_PTR& aNode2,
                                      unsigned int aDistance = 0 );

    /**
     * Function RemoveConnection()
     * Removes a connection described by a given edge pointer.
     * @param aEdge is a pointer to edge to be removed.
     */
    void RemoveConnection( const RN_EDGE_PTR& aEdge )
    {
        m_edges.remove( aEdge );
    }

    /**
     * Function GetConnections()
     * Returns the list of edges that currently connect nodes.
     * @return the list of edges that currently connect nodes.
     */
    const RN_EDGE_LIST& GetConnections() const
    {
        return m_edges;
    }

protected:
    ///> Set of nodes that are used are expected to be connected together.
    RN_NODE_SET m_nodes;

    ///> List of edges that currently connect nodes.
    RN_EDGE_LIST m_edges;
};


/**
 * Class RN_POLY
 * Describes a single subpolygon (ZONE_CONTAINER is supposed to contain one or more of those) and
 * performs fast point-inside-polygon test.
 */
class RN_POLY
{
public:
    RN_POLY( const CPolyPt* aBegin, const CPolyPt* aEnd, const ZONE_CONTAINER* aParent,
          RN_LINKS& aConnections, const BOX2I& aBBox );

    /**
     * Function GetNode()
     * Returns node representing a polygon (it has the same coordinates as the first point of its
     * bounding polyline.
     */
    const RN_NODE_PTR& GetNode() const
    {
        return m_node;
    }


    /**
     * Function GetParent()
     * Returns pointer to zone that is the owner of subpolygon.
     * @return Pointer to zone that is the owner of subpolygon.
     */
    const ZONE_CONTAINER* GetParent() const
    {
        return m_parent;
    }

    /**
     * Function HitTest()
     * Tests if selected node is located within polygon boundaries.
     * @param aNode is a node to be checked.
     * @return True is the node is located within polygon boundaries.
     */
    bool HitTest( const RN_NODE_PTR& aNode ) const;

private:
    ///> Owner of this subpolygon.
    const ZONE_CONTAINER* m_parent;

    ///> Pointer to the first point of polyline bounding the polygon.
    const CPolyPt* m_begin;

    ///> Pointer to the last point of polyline bounding the polygon.
    const CPolyPt* m_end;

    ///> Bounding box of the polygon.
    BOX2I m_bbox;

    ///> Node representing a polygon (it has the same coordinates as the first point of its
    ///> bounding polyline.
    RN_NODE_PTR m_node;

    friend bool sortArea( const RN_POLY& aP1, const RN_POLY& aP2 );
};


/**
 * Class RN_NET
 * Describes ratsnest for a single net.
 */
class RN_NET
{
public:
    ///> Default constructor.
    RN_NET() : m_dirty( true ), m_visible( true )
    {}

    /**
     * Function SetVisible()
     * Sets state of the visibility flag.
     * @param aEnabled is new state. True if ratsnest for a given net is meant to be displayed,
     * false otherwise.
     */
    void SetVisible( bool aEnabled )
    {
        m_visible = aEnabled;
    }

    /**
     * Function IsVisible()
     * Returns the visibility flag state.
     * @return True if ratsnest for given net is set as visible, false otherwise,
     */
    bool IsVisible() const
    {
        return m_visible;
    }

    /**
     * Function MarkDirty()
     * Marks ratsnest for given net as 'dirty', ie. requiring recomputation.
     */
    void MarkDirty()
    {
        m_dirty = true;
    }

    /**
     * Function IsDirty()
     * Returns state of the 'dirty' flag, indicating that ratsnest for a given net is invalid
     * and requires an update.
     * @return True if ratsnest requires recomputation, false otherwise.
     */
    bool IsDirty() const
    {
        return m_dirty;
    }

    /**
     * Function GetUnconnected()
     * Returns pointer to a vector of edges that makes ratsnest for a given net.
     * @return Pointer to a vector of edges that makes ratsnest for a given net.
     */
    const std::vector<RN_EDGE_PTR>* GetUnconnected() const
    {
        return m_rnEdges.get();
    }

    /**
     * Function Update()
     * Recomputes ratsnest for a net.
     */
    void Update();

    /**
     * Function AddItem()
     * Adds an appropriate node associated with selected pad, so it is
     * taken into account during ratsnest computations.
     * @param aPad is a pad for which node is added.
     */
    void AddItem( const D_PAD* aPad );

    /**
     * Function AddItem()
     * Adds an appropriate node associated with selected via, so it is
     * taken into account during ratsnest computations.
     * @param aVia is a via for which node is added.
     */
    void AddItem( const SEGVIA* aVia );

    /**
     * Function AddItem()
     * Adds appropriate nodes and edges associated with selected track, so they are
     * taken into account during ratsnest computations.
     * @param aTrack is a track for which nodes and edges are added.
     */
    void AddItem( const TRACK* aTrack );

    /**
     * Function AddItem()
     * Processes zone to split it into subpolygons and adds appropriate nodes for them, so they are
     * taken into account during ratsnest computations.
     * @param aZone is a zone to be processed.
     */
    void AddItem( const ZONE_CONTAINER* aZone );

    /**
     * Function RemoveItem()
     * Removes all nodes and edges associated with selected pad, so they are not
     * taken into account during ratsnest computations anymore.
     * @param aPad is a pad for which nodes and edges are removed.
     */
    void RemoveItem( const D_PAD* aPad );

    /**
     * Function RemoveItem()
     * Removes all nodes and edges associated with selected via, so they are not
     * taken into account during ratsnest computations anymore.
     * @param aVia is a via for which nodes and edges are removed.
     */
    void RemoveItem( const SEGVIA* aVia );

    /**
     * Function RemoveItem()
     * Removes all nodes and edges associated with selected track, so they are not
     * taken into account during ratsnest computations anymore.
     * @param aTrack is a track for which nodes and edges are removed.
     */
    void RemoveItem( const TRACK* aTrack );

    /**
     * Function RemoveItem()
     * Removes all nodes and edges associated with selected zone, so they are not
     * taken into account during ratsnest computations anymore.
     * @param aZone is a zone for which nodes and edges are removed.
     */
    void RemoveItem( const ZONE_CONTAINER* aZone );

    /**
     * Function GetNodes()
     * Returns list of nodes that are associated with a given item.
     * @param aItem is an item for which the list is generated.
     * @return List of associated nodes.
     */
    std::list<RN_NODE_PTR> GetNodes( const BOARD_CONNECTED_ITEM* aItem ) const;

    /**
     * Function GetClosestNode()
     * Returns a single node that lies in the shortest distance from a specific node.
     * @param aNode is the node for which the closest node is searched.
     */
    const RN_NODE_PTR GetClosestNode( const RN_NODE_PTR& aNode ) const;

    /**
     * Function GetClosestNode()
     * Returns a single node that lies in the shortest distance from a specific node and meets
     * selected filter criterion..
     * @param aNode is the node for which the closest node is searched.
     * @param aFilter is a functor that filters nodes.
     */
    const RN_NODE_PTR GetClosestNode( const RN_NODE_PTR& aNode,
                                      const RN_NODE_FILTER& aFilter ) const;

    /**
     * Function GetClosestNodes()
     * Returns list of nodes sorted by the distance from a specific node.
     * @param aNode is the node for which the closest nodes are searched.
     * @param aNumber is asked number of returned nodes. If it is negative then all nodes that
     * belong to the same net are returned. If asked number is greater than number of possible
     * nodes then the size of list is limited to number of possible nodes.
     */
    std::list<RN_NODE_PTR> GetClosestNodes( const RN_NODE_PTR& aNode, int aNumber = -1 ) const;

    /**
     * Function GetClosestNodes()
     * Returns filtered list of nodes sorted by the distance from a specific node.
     * @param aNode is the node for which the closest nodes are searched.
     * @param aFilter is a functor that filters nodes.
     * @param aNumber is asked number of returned nodes. If it is negative then all nodes that
     * belong to the same net are returned. If asked number is greater than number of possible
     * nodes then the size of list is limited to number of possible nodes.
     */
    std::list<RN_NODE_PTR> GetClosestNodes( const RN_NODE_PTR& aNode,
                                            const RN_NODE_FILTER& aFilter, int aNumber = -1 ) const;

    /**
     * Function GetEdges()
     * Returns pointer to the vector of edges that makes ratsnest for a given net.
     * @return Pointer to the vector of edges that makes ratsnest for a given net
     */
    const std::vector<RN_EDGE_PTR>* GetEdges() const
    {
        return m_rnEdges.get();
    }

    /**
     * Function AddSimpleNode()
     * Changes drawing mode for a node to simple (ie. one ratsnest line per node).
     * @param aNode is a node that changes its drawing mode..
     */
    void AddSimpleNode( RN_NODE_PTR& aNode )
    {
        m_simpleNodes.push_back( aNode );
        aNode->SetFlag( true );
    }

    /**
     * Function GetSimpleNodes()
     * Returns list of nodes for which ratsnest is drawn in simple mode (ie. one
     * ratsnest line per node).
     * @return list of nodes for which ratsnest is drawn in simple mode.
     */
    const std::deque<RN_NODE_PTR>& GetSimpleNodes() const
    {
        return m_simpleNodes;
    }

    /**
     * Function ClearSimple()
     * Removes all nodes and edges that are used for displaying ratsnest in simple mode.
     */
    void ClearSimple();

protected:
    ///> Validates edge, ie. modifies source and target nodes for an edge
    ///> to make sure that they are not ones with the flag set.
    void validateEdge( RN_EDGE_PTR& aEdge );

    ///> Removes all ratsnest edges for a given node.
    void clearNode( const RN_NODE_PTR& aNode );

    ///> Adds appropriate edges for nodes that are connected by zones.
    void processZones();

    ///> Recomputes ratsnset from scratch.
    void compute();

    ////> Stores information about connections for a given net.
    RN_LINKS m_links;

    ///> Vector of edges that makes ratsnest for a given net.
    boost::shared_ptr< std::vector<RN_EDGE_PTR> > m_rnEdges;

    ///> List of nodes for which ratsnest is drawn in simple mode.
    std::deque<RN_NODE_PTR> m_simpleNodes;

    ///> Flag indicating necessity of recalculation of ratsnest for a net.
    bool m_dirty;

    ///> Map that associates nodes in the ratsnest model to respective nodes.
    boost::unordered_map<const D_PAD*, RN_NODE_PTR> m_pads;

    ///> Map that associates nodes in the ratsnest model to respective vias.
    boost::unordered_map<const SEGVIA*, RN_NODE_PTR> m_vias;

    ///> Map that associates edges in the ratsnest model to respective tracks.
    boost::unordered_map<const TRACK*, RN_EDGE_PTR> m_tracks;

    ///> Map that associates groups of subpolygons in the ratsnest model to their respective zones.
    boost::unordered_map<const ZONE_CONTAINER*, std::deque<RN_POLY> > m_zonePolygons;

    ///> Map that associates groups of edges in the ratsnest model to their respective zones.
    boost::unordered_map<const ZONE_CONTAINER*, std::deque<RN_EDGE_PTR> > m_zoneConnections;

    ///> Visibility flag.
    bool m_visible;
};


/**
 * Class RN_DATA
 *
 * Stores information about unconnected items for the whole PCB.
 */
class RN_DATA
{
public:
    /**
     * Default constructor
     * @param aBoard is the board to be processed in order to look for unconnected items.
     */
    RN_DATA( const BOARD* aBoard ) : m_board( aBoard ) {}

    /**
     * Function UpdateItem()
     * Updates ratsnest data for an item.
     * @param aItem is an item to be updated.
     */
    void Update( const BOARD_CONNECTED_ITEM* aItem );

    /**
     * Function UpdateItem()
     * Updates ratsnest data for a module.
     * @param aItem is a module to be updated.
     */
    void Update( const MODULE* aModule );

    /**
     * Function AddSimple()
     * Sets an item to be drawn in simple mode (ie. one line per node, instead of full ratsnest).
     * It is used for drawing temporary ratsnest, eg. while moving an item.
     * @param aItem is an item to be drawn in simple node.
     */
    void AddSimple( const BOARD_CONNECTED_ITEM* aItem );

    /**
     * Function AddSimple()
     * Sets a module to be drawn in simple mode (ie. one line per node, instead of full ratsnest).
     * It is used for drawing temporary ratsnest, eg. while moving a module.
     * @param aModule is a module to be drawn in simple node.
     */
    void AddSimple( const MODULE* aModule );

    /**
     * Function ClearSimple()
     * Clears the list of nodes for which ratsnest is drawn in simple mode (one line per node).
     */
    void ClearSimple();

    /**
     * Function ProcessBoard()
     * Prepares data for computing (computes a list of current nodes and connections). It is
     * required to run only once after loading a board.
     */
    void ProcessBoard();

    /**
     * Function Recalculate()
     * Recomputes ratsnest for selected net number or all nets that need updating.
     * @param aNet is a net number. If it is negative, all nets that need updating are recomputed.
     */
    void Recalculate( int aNet = -1 );

    /**
     * Function GetNets()
     * Returns ratsnest grouped by net numbers.
     * @return Vector of ratsnest grouped by net numbers.
     */
    const std::vector<RN_NET>& GetNets() const
    {
        return m_nets;
    }

protected:
    /**
     * Function updateNet()
     * Recomputes ratsnest for a single net.
     * @param aNetCode is the net number to be recomputed.
     */
    void updateNet( int aNetCode );

    ///> Board to be processed.
    const BOARD* m_board;

    ///> Stores information about ratsnest grouped by net numbers.
    std::vector<RN_NET> m_nets;
};

#endif /* RATSNEST_DATA_H */
