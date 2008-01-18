/***************************************************************************
                              qgssnapper.cpp    
                              --------------
  begin                : June 7, 2007
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

#include "qgssnapper.h"
#include "qgsmaprender.h"
#include "qgsmaptopixel.h"
#include "qgsvectorlayer.h"
#include <QMultiMap>
#include <cmath>


QgsSnapper::QgsSnapper(QgsMapRender* mapRender): mMapRender(mapRender)
{

}

QgsSnapper::QgsSnapper()
{
  
}

QgsSnapper::~QgsSnapper()
{
  
}

int QgsSnapper::snapPoint(const QPoint& startPoint, QList<QgsSnappingResult>& snappingResult)
{
  snappingResult.clear();

  //list must have the same length
  if(!(mLayersToSnap.size() == mSnappingTolerances.size() && mLayersToSnap.size() == mSnapToList.size()))
    {
      return 1;
    }

  QList<QgsVectorLayer*>::iterator layerIt = mLayersToSnap.begin();
  QList<double>::const_iterator  toleranceIt = mSnappingTolerances.constBegin();
  QList<QgsSnapper::SNAP_TO>::const_iterator snapToIt = mSnapToList.constBegin();

  QMultiMap<double, QgsSnappingResult> snappingResultList;//all snapping results
  QMultiMap<double, QgsSnappingResult> currentResultList; //snapping results of examined layer

  //start point in (output) map coordinates
  QgsPoint mapCoordPoint = mMapRender->coordXForm()->toMapCoordinates(startPoint.x(), startPoint.y()); 
  QgsPoint layerCoordPoint; //start point in layer coordinates
  QgsSnappingResult newResult;


  for(; layerIt != mLayersToSnap.end(); ++layerIt, ++toleranceIt, ++snapToIt)
    {
      //transform point from map coordinates to layer coordinates
      layerCoordPoint = mMapRender->outputCoordsToLayerCoords(*layerIt, mapCoordPoint);
      if( (*layerIt)->snapWithContext(layerCoordPoint, *toleranceIt, currentResultList, *snapToIt) != 0)
	{
	  //error
	}
  
      //transform each result from layer crs to map crs (including distance)
      QMultiMap<double, QgsSnappingResult>::iterator currentResultIt;
      for(currentResultIt = currentResultList.begin(); currentResultIt != currentResultList.end(); ++currentResultIt)
	{
	  //for each snapping result: transform start point, snap point and other points into map coordinates to find out distance
	  //store results in snapping result list
	  newResult = currentResultIt.value();
	  newResult.snappedVertex = mMapRender->layerCoordsToOutputCoords(*layerIt, currentResultIt.value().snappedVertex);
	  newResult.beforeVertex = mMapRender->layerCoordsToOutputCoords(*layerIt, currentResultIt.value().beforeVertex);
	  newResult.afterVertex = mMapRender->layerCoordsToOutputCoords(*layerIt, currentResultIt.value().afterVertex);
	  snappingResultList.insert(sqrt(newResult.snappedVertex.sqrDist(mapCoordPoint)), newResult);
	}
    }
  
  //evaluate results according to snap mode
  QMultiMap<double, QgsSnappingResult>::iterator evalIt =  snappingResultList.begin();
  if(evalIt == snappingResultList.end())
    {
      return 0;
    }

  if(mSnapMode == QgsSnapper::ONE_RESULT)
    {
      //return only closest result
      snappingResult.push_back(evalIt.value());
    }
  else if(mSnapMode == QgsSnapper::SEVERAL_RESULTS_SAME_POSITION)
    {
      
      double minDistance = evalIt.key();
      QList<QgsSnappingResult> values = snappingResultList.values(minDistance);
      for (int i = 0; i < values.size(); ++i)
	{
	  snappingResult.push_back(values.at(i));
	}
    }
  else //take all results
    {
      for(;evalIt != snappingResultList.end(); ++evalIt)
	{
	  snappingResult.push_back(evalIt.value());
	}
    }

  return 0;
}

void QgsSnapper::setLayersToSnap(const QList<QgsVectorLayer*>& layerList)
{
  mLayersToSnap = layerList;
}

void QgsSnapper::setTolerances(const QList<double>& toleranceList)
{
  mSnappingTolerances = toleranceList;
}

void QgsSnapper::setSnapToList(const QList<QgsSnapper::SNAP_TO>& snapToList)
{
  mSnapToList = snapToList;
}

void QgsSnapper::setSnapMode(QgsSnapper::SNAP_MODE snapMode)
{
  mSnapMode = snapMode;
}
