/***************************************************************************
                         qgsoverlayobject.cpp  -  description
                         ------------------------------
    begin                : January 2007
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

#include "qgsoverlayobject.h"

QgsOverlayObject::QgsOverlayObject(int height, int width, const QgsPoint* p, bool fixed, unsigned char* wkb, int geometryNr): mHeight(height), mWidth(width), mFixed(fixed), mWKB(wkb), mGeometryNr(geometryNr)
{
  if(p)
    {
      mPosition = *p;
    }
}

QgsOverlayObject::~QgsOverlayObject()
{
  delete mWKB;
}

void QgsOverlayObject::setWkb(unsigned char* wkb)
{
  delete mWKB;
  mWKB = wkb;
}


