/***************************************************************************
                          qgsvectorlayer.h  -  description
                             -------------------
    begin                : Wed Jun 26 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
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

#ifndef QGSVECTORLAYER_H
#define QGSVECTORLAYER_H
class QPainter;
class QgsRect;
class QgsCoordinateTransform;
class OGRLayer;
class OGRDataSource;

#include "qgsmaplayer.h"
#include "qvaluevector.h"
#include "qgsattributetabledisplay.h"

/*! \class QgsVectorLayer
 * \brief Vector layer backed by a data source provider
 */

class QgsVectorLayer:public QgsMapLayer
{
  Q_OBJECT public:

	//! Constructor
	  QgsVectorLayer(QString baseName = 0, QString path = 0);
	//! Destructor
	 ~QgsVectorLayer();
	//! Identify feature found within the search rectangle
	void identify(QgsRect *);
	//! Select features found within the search rectangle
	void select(QgsRect* rect, bool lock);
	//! Display the attribute table
	void table();
	enum SHAPETYPE
	{
		Point,
		Line,
		Polygon
	};

 public slots:
     /**Sets the 'tabledisplay' to 0 again*/
     void invalidateTableDisplay();
     void select(int number);
     void removeSelection();
     void triggerRepaint();

 protected:
        /**Pointer to the table display object if there is one, else a pointer to 0*/
        QgsAttributeTableDisplay* tabledisplay;
	/**Vector holding the information which features are activated*/
	QValueVector<bool>* selected;
	/**Color to draw and fill the selected features*/
	QColor selectionColor;

  private:						// Private attributes
//! Draws the layer using coordinate transformation
	void draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf);
	//! Pointer to data provider derived from the abastract base class QgsDataProvider
	QgsDataProvider *dataSource;

  /**  */
	bool registered;
	enum ENDIAN
	{
		NDR = 1,
		XDR = 0
	};
	enum WKBTYPE
	{
		WKBPoint = 1,
		WKBLineString,
		WKBPolygon,
		WKBMultiPoint,
		WKBMultiLineString,
		WKBMultiPolygon
	};
  private:						// Private methods
  /** No descriptions */
	void registerFormats();
	int endian();

 signals:
	void repaintRequested();

};

#endif
