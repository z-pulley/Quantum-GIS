/***************************************************************************
                          qgsmaplayerregistry.h    
           Singleton class for keeping track of loaded layers
                             -------------------
    begin                : Sun June 04 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */
 
#ifndef QGSMAPLAYERREGISTRY_H
#define QGSMAPLAYERREGISTRY_H
#include <map>
#include <qobject.h>
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"

class QString;
class QStringList;

/**
* \class QgsMapLayerRegistry
* \brief This class tracks map layers that are currently loaded an provides
* a means to fetch a pointer to a map layer and delete it
*/
class QgsMapLayerRegistry : public QObject
{
   Q_OBJECT;

public:

 //! Returns the instance pointer, creating the object on the first call
 static QgsMapLayerRegistry * instance();

 //! Retrieve a pointer to a loaded plugin by id
 QgsMapLayer * mapLayer(QString theLayerId);

 //! Retrieve the mapLayers collection (mainly intended for use by projectio)
 std::map<QString,QgsMapLayer*> mapLayers();

 /** Add a layer to the map of loaded layers 
    @returns NULL if unable to add layer, otherwise pointer to newly added layer
 */
 QgsMapLayer *  addMapLayer(QgsMapLayer * theMapLayer);

 //! Remove a layer from qgis - any canvases using that layer will need to remove it
 void removeMapLayer(QString theLayerId);

 //! Remove all registered layers 
 void removeAllMapLayers();

 //! Get a vector layer from the registry - the the requested key does not exist or
 //does not correspond to a vector layer, null returned!
 QgsVectorLayer * getVectorLayer(QString theLayerId);

signals:

    /** emitted when a layer is removed from the registry

       connected to main map canvas and overview map canvas remove()
    */
 void layerWillBeRemoved(QString theLayerId);

    /** emitted when a layer is added to the registry

       connected to main map canvas and overview map canvas addLayer()
    */
 void layerWasAdded(QgsMapLayer * theMapLayer);

protected:

 //! protected constructor
 QgsMapLayerRegistry( QObject * parent = 0, const char * name = 0 );

private:

 static QgsMapLayerRegistry* mInstance;

 std::map<QString,QgsMapLayer*> mMapLayers;

  /** debugging member
      invoked when a connect() is made to this object 
  */
  void connectNotify( const char * signal );


}; // class QgsMapLayerRegistry

#endif //QgsMapLayerRegistry_H

