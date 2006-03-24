/***************************************************************************
    qgsmaptoolvertexedit.h  - tool for adding, moving, deleting vertices
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSMAPTOOLVERTEXEDIT_H
#define QGSMAPTOOLVERTEXEDIT_H

#include "qgsmaptool.h"
#include "qgsgeometry.h"

class QgsRubberBand;
class QgsVertexMarker;

#define MapTool_VertexEdit "vertex edit"

class QgsMapToolVertexEdit : public QgsMapTool
{
  public:
  
    enum Tool
    {
      AddVertex,
      MoveVertex,
      DeleteVertex
    };
  
    //! constructor
    QgsMapToolVertexEdit(QgsMapCanvas* canvas, enum Tool tool);

    virtual ~QgsMapToolVertexEdit();
    
    //! Overridden mouse move event
    virtual void canvasMoveEvent(QMouseEvent * e);
  
    //! Overridden mouse press event
    virtual void canvasPressEvent(QMouseEvent * e);
  
    //! Overridden mouse release event
    virtual void canvasReleaseEvent(QMouseEvent * e);

    virtual QString toolName() { return MapTool_VertexEdit; }

    virtual void deactivate();

  private:
    
    //! create instance of rubber band
    void createRubberBand();
    
    //! returns tolerance in map coordinates
    double tolerance();
    
    //! current vertex edit tool
    enum Tool mTool;
    
    bool snapVertexWithContext(QgsPoint& point);
    
    bool snapSegmentWithContext(QgsPoint& point);
    
    //! The snapped-to segment before this vertex number (identifying the vertex that is being moved)
    QgsGeometryVertexIndex mSnappedAtVertex;

    //! The snapped-to segment before this vertex number (identifying the segment that a new vertex is being added to)
    QgsGeometryVertexIndex mSnappedBeforeVertex;

    //! The snapped-to feature ID
    int mSnappedAtFeatureId;

    //! The snapped-to geometry
    QgsGeometry mSnappedAtGeometry;
    
    QgsRubberBand* mRubberBand;
    
    bool mStartPointValid;
    bool mStopPointValid;
    
    QgsVertexMarker* mCross;

};

#endif
