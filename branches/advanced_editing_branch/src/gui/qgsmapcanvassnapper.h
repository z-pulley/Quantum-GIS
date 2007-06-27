/***************************************************************************
                              qgsmapcanvassnapper.h    
                              ---------------------
  begin                : June 21, 2007
  copyright            : (C) 2007 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPCANVASSNAPPER_H
#define QGSMAPCANVASSNAPPER_H

#include "qgssnapper.h"
#include <QList>

class QgsMapCanvas;
class QPoint;
class QgsSnapper;

/**This class reads the snapping properties from the 
 current project and configures a QgsSnapper to perform the snapping.
 Snapping can be done to the active layer  (usefull for selecting a vertex to manipulate) 
 or to background layers*/
class QgsMapCanvasSnapper
{
 public:
  QgsMapCanvasSnapper(QgsMapCanvas* canvas);

  QgsMapCanvasSnapper();
  
  ~QgsMapCanvasSnapper();

  /**Does a snap to select a vertex of the current layer. Uses snap mode 
     QgsSnapper::SEVERAL_RESULTS_SAME_POSITION if topological editing is enabled 
     and QgsSnapper::ONE_RESULT_BY_SEGMENT if not.
     @param p start point of the snap (in pixel coordinates)
     @param results list to which the results are appended*/
  int snapToVertexCurrentLayer(const QPoint& p, QList<QgsSnappingResult>& results);
  /**Snaps to the background layers. This method is usefull to align the features of the 
     edited layers to those of other layers (as described in the project properties). 
     Uses snap mode QgsSnapper::ONE_RESULT. Therefore, only the
     closest result is returned.
     @param p start point of the snap (in pixel coordinates)
     @param result snapped point
     @return 0 in case of success*/
  int snapToBackgroundLayers(const QPoint& p, QgsPoint& result);

  void setMapCanvas(QgsMapCanvas* canvas);

 private:
  /**Pointer to the map canvas*/
  QgsMapCanvas* mMapCanvas;
  /**The object which does the snapping operations*/
  QgsSnapper* mSnapper;
};

#endif 
