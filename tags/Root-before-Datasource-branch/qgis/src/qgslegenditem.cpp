/***************************************************************************
                          qgslegenditem.cpp  -  description
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
               Romans 3:23=>Romans 6:23=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qlabel.h>
#include <qcheckbox.h>
#include "qgssymbol.h"
#include "qgsmaplayer.h"
#include "qgslegenditem.h"


QgsLegendItem::QgsLegendItem(QgsMapLayer * lyr, QListView * parent)
:QCheckListItem(parent, lyr->name(), QCheckListItem::CheckBox), m_layer(lyr)
{
	setOn(lyr->visible());


}

QgsLegendItem::~QgsLegendItem()
{
}

/** Write property of QString layerName. */
void QgsLegendItem::setLayerName(const QString & _newVal)
{
	layerName = _newVal;
}

/** Write property of QString displayName. */
void QgsLegendItem::setDisplayName(const QString & _newVal)
{
	displayName = _newVal;
}

void QgsLegendItem::stateChange(bool vis)
{
	m_layer->setVisible(vis);
}

QgsMapLayer *QgsLegendItem::layer()
{
	return m_layer;
}
