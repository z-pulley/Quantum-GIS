/***************************************************************************
  plugin.cpp
  Plugin to draw scale bar on map
Functions:

-------------------
begin                : Jun 1, 2004
copyright            : (C) 2004 by Peter Brewer
email                : sbr00pwb@users.sourceforge.net

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

// includes

#include <qgisapp.h>
#include "qgisgui.h"
#include <qgsmaplayer.h>
#include "plugin.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include <qgspoint.h>
#include <qgsmaptopixel.h>

#include <QPainter>
#include <QAction>
#include <QPen>
#include <QPolygon>
#include <QString>
#include <QFontMetrics>
#include <QFont>
#include <QColor>
#include <QMenu>

//non qt includes
#include <iostream>
#include <cmath>

//the gui subclass
#include "plugingui.h"

// xpm for creating the toolbar icon
#include "icon.xpm"
//
#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

static const char * const ident_ = "$Id$";

static const char * const name_ = "ScaleBar";
static const char * const description_ = "Plugin to draw scale bar on map";
static const char * const version_ = "Version 0.1";
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsScaleBarPlugin::QgsScaleBarPlugin(QgisApp * theQGisApp,
                                     QgisIface * theQgisInterFace):
        QgisPlugin(name_,description_,version_,type_),
        qgisMainWindowPointer(theQGisApp),
        qGisInterface(theQgisInterFace)
{
  mPlacementLabels << tr("Top Left") << tr("Bottom Left") << tr("Top Right") << tr("Bottom Right");
  mStyleLabels << tr("Tick Down") << tr("Tick Up") << tr("Bar") << tr("Box");

  mPreferredSize = 30;
  mPlacementIndex = 0;
  mStyleIndex = 0;
  mEnabled = true;
  mSnapping = true;
  mColour = Qt::black;
}

QgsScaleBarPlugin::~QgsScaleBarPlugin()
{

}

/*
 * Initialize the GUI interface for the plugin
 */
void QgsScaleBarPlugin::initGui()
{
  QMenu *pluginMenu = qGisInterface->getPluginMenu(tr("&Decorations"));
  menuId = pluginMenu->insertItem(QIcon(icon),tr("&ScaleBar"), this, SLOT(run()));

  pluginMenu->setWhatsThis(menuId, tr("Creates a scale bar that is displayed on the map canvas"));

  // Create the action for tool
#if QT_VERSION < 0x040000
  myQActionPointer = new QAction(tr("Scale Bar"), QIcon(icon), "&Wmi",0, this, "run");
#else
  myQActionPointer = new QAction(QIcon(icon), tr("Scale Bar"), this);
#endif
  myQActionPointer->setWhatsThis(tr("Creates a scale bar that is displayed on the map canvas"));
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  //render the scale bar each time the map is rendered
  connect(qGisInterface->getMapCanvas(), SIGNAL(renderComplete(QPainter *)), this, SLOT(renderScaleBar(QPainter *)));
  //this resets this plugin up if a project is loaded
  connect(qgisMainWindowPointer, SIGNAL(projectRead()), this, SLOT(projectRead()));
  // Add the icon to the toolbar
  qGisInterface->addToolBarIcon(myQActionPointer);
}

void QgsScaleBarPlugin::projectRead()
{
#ifdef QGISDEBUG
    std::cout << "+++++++++ scalebar plugin - project read slot called...." << std::endl;
#endif    //default text to start with - try to fetch it from qgsproject


    mPreferredSize = QgsProject::instance()->readNumEntry("ScaleBar","/PreferredSize",30);
    mStyleIndex = QgsProject::instance()->readNumEntry("ScaleBar","/Style",0);
    mPlacementIndex = QgsProject::instance()->readNumEntry("ScaleBar","/Placement",0);
    mEnabled = QgsProject::instance()->readBoolEntry("ScaleBar","/Enabled",true);
    mSnapping = QgsProject::instance()->readBoolEntry("ScaleBar","/Snapping",true);
    int myRedInt = QgsProject::instance()->readNumEntry("ScaleBar","/ColorRedPart",0);
    int myGreenInt = QgsProject::instance()->readNumEntry("ScaleBar","/ColorGreenPart",0);
    int myBlueInt = QgsProject::instance()->readNumEntry("ScaleBar","/ColorBluePart",0);
    mColour = QColor(myRedInt,myGreenInt,myBlueInt);
}
//method defined in interface
void QgsScaleBarPlugin::help()
{
  //implement me!
}

// Slot called when the  menu item is activated
void QgsScaleBarPlugin::run()
{
  QgsScaleBarPluginGui *myPluginGui=new QgsScaleBarPluginGui(qgisMainWindowPointer, QgisGui::ModalDialogFlags);
  myPluginGui->setPreferredSize(mPreferredSize);
  myPluginGui->setSnapping(mSnapping);
  myPluginGui->setPlacementLabels(mPlacementLabels);
  myPluginGui->setPlacement(mPlacementIndex);
  myPluginGui->setEnabled(mEnabled);
  myPluginGui->setStyleLabels(mStyleLabels);
  myPluginGui->setStyle(mStyleIndex);
  myPluginGui->setColour(mColour);

  connect(myPluginGui, SIGNAL(changePreferredSize(int)), this, SLOT(setPreferredSize(int)));
  connect(myPluginGui, SIGNAL(changeSnapping(bool)), this, SLOT(setSnapping(bool)));
  connect(myPluginGui, SIGNAL(changePlacement(int)), this, SLOT(setPlacement(int)));
  connect(myPluginGui, SIGNAL(changeEnabled(bool)), this, SLOT(setEnabled(bool)));
  connect(myPluginGui, SIGNAL(changeStyle(int)), this, SLOT(setStyle(int)));
  connect(myPluginGui, SIGNAL(changeColour(QColor)), this, SLOT(setColour(QColor)));
  connect(myPluginGui, SIGNAL(refreshCanvas()), this, SLOT(refreshCanvas()));
  myPluginGui->show();
  //set the map units in the spin box
  int myUnits=qGisInterface->getMapCanvas()->mapUnits();
  switch (myUnits)
  {
      case 0: myPluginGui->getSpinSize()->setSuffix(tr(" metres/km")); break;
      case 1: myPluginGui->getSpinSize()->setSuffix(tr(" feet")); break;
      case 2: myPluginGui->getSpinSize()->setSuffix(tr(" degrees")); break;
      default: std::cout << "Error: not picked up map units - actual value = " << myUnits << std::endl;
  };
}


void QgsScaleBarPlugin::refreshCanvas()
{
  qGisInterface->getMapCanvas()->refresh();
}



// Actual drawing of Scale Bar
void QgsScaleBarPlugin::renderScaleBar(QPainter * theQPainter)
{
  int myBufferSize=1; //softcode this later

  //Get canvas dimensions
  int myCanvasHeight = theQPainter->device()->height();
  int myCanvasWidth = theQPainter->device()->width();

  //Get map units per pixel. This can be negative at times (to do with
  //projections) and that just confuses the rest of the code in this
  //function, so force to a positive number.
  double myMuppDouble = std::abs(qGisInterface->getMapCanvas()->mupp());

  // Exit if the canvas width is 0 or layercount is 0 or QGIS will freeze
  int myLayerCount=qGisInterface->getMapCanvas()->layerCount();
  if (!myLayerCount || !myCanvasWidth || !myMuppDouble) return;

  //Large if statement which determines whether to render the scale bar
  if (mEnabled)
  {
    // Hard coded sizes
    int myMajorTickSize=8;
    int myTextOffsetX=3;
    double myActualSize=mPreferredSize;
    int myMargin=20;

    //Calculate size of scale bar for preferred number of map units
    double myScaleBarWidth = mPreferredSize / myMuppDouble;

    //If scale bar is very small reset to 1/4 of the canvas wide
    if (myScaleBarWidth < 30)
    {
      myScaleBarWidth = myCanvasWidth / 4; // pixels
      myActualSize = myScaleBarWidth * myMuppDouble; // map units
    };

    //if scale bar is more than half the canvas wide keep halving until not
    while (myScaleBarWidth > myCanvasWidth/3)
    {
      myScaleBarWidth = myScaleBarWidth /3;
    };
    myActualSize = myScaleBarWidth * myMuppDouble;

    // Work out the exponent for the number - e.g, 1234 will give 3,
    // and .001234 will give -3
    double myPowerOf10 = floor(log10(myActualSize));

    // snap to integer < 10 times power of 10
    if (mSnapping) 
    {
      double scaler = pow(10.0, myPowerOf10);
      myActualSize = round(myActualSize / scaler) * scaler;
      myScaleBarWidth = myActualSize / myMuppDouble;
    }

    //Get type of map units and set scale bar unit label text
    QGis::units myMapUnits=qGisInterface->getMapCanvas()->mapUnits();
    QString myScaleBarUnitLabel;
    switch (myMapUnits)
    {
    case QGis::METERS: 
      if (myActualSize > 1000.0)
      {
	myScaleBarUnitLabel=tr(" km");
	myActualSize = myActualSize/1000;
      }
      else if (myActualSize < 0.01)
      {
	myScaleBarUnitLabel=tr(" mm");
	myActualSize = myActualSize*1000;
      }
      else if (myActualSize < 0.1)
      {
        myScaleBarUnitLabel=tr(" cm");
        myActualSize = myActualSize*100;
      }
      else
	myScaleBarUnitLabel=tr(" m"); 
      break;
    case QGis::FEET:
      if (myActualSize == 1.0)
	myScaleBarUnitLabel=tr(" foot"); 
      else
	myScaleBarUnitLabel=tr(" feet"); 
      break;
    case QGis::DEGREES:
      if (myActualSize == 1.0)
	myScaleBarUnitLabel=tr(" degree"); 
      else
	myScaleBarUnitLabel=tr(" degrees"); 
      break;
    case QGis::UNKNOWN:
      myScaleBarUnitLabel=tr(" unknown");
    default: 
      std::cout << "Error: not picked up map units - actual value = " 
		<< myMapUnits << std::endl;
    };

    //Set font and calculate width of unit label
    int myFontSize = 10; //we use this later for buffering
    QFont myFont( "helvetica", myFontSize );
    theQPainter->setFont(myFont);
    QFontMetrics myFontMetrics( myFont );
    double myFontWidth = myFontMetrics.width( myScaleBarUnitLabel );
    double myFontHeight = myFontMetrics.height();

    //Set the maximum label
    QString myScaleBarMaxLabel=QString::number(myActualSize);

    //Calculate total width of scale bar and label
    double myTotalScaleBarWidth = myScaleBarWidth + myFontWidth;

    //determine the origin of scale bar depending on placement selected
    int myOriginX=myMargin;
    int myOriginY=myMargin;
    switch (mPlacementIndex)
    {
    case 0: // Top Left
      myOriginX=myMargin;
      myOriginY=myMargin;
      break;
    case 1: // Bottom Left
      myOriginX=myMargin;
      myOriginY=myCanvasHeight - myMargin;
      break;
    case 2: // Top Right
      myOriginX=myCanvasWidth - ((int) myTotalScaleBarWidth) - myMargin;
      myOriginY=myMargin;
      break;
    case 3: // Bottom Right
      myOriginX=myCanvasWidth - ((int) myTotalScaleBarWidth) - myMargin;
      myOriginY=myCanvasHeight - myMargin;
      break;
    default:
      std::cout << "Unable to determine where to put scale bar so defaulting to top left" << std::endl;
    }

    //Set pen to draw with
    QPen myForegroundPen( mColour, 2 );
    QPen myBackgroundPen( Qt::white, 4 );

    //Cast myScaleBarWidth to int for drawing
    int myScaleBarWidthInt = (int) myScaleBarWidth;

    //Create array of vertices for scale bar depending on style
    switch (mStyleIndex)
    {
    case 0: // Tick Down
    {
      QPolygon myTickDownArray(4);
      //draw a buffer first so bar shows up on dark images
      theQPainter->setPen( myBackgroundPen );
      myTickDownArray.putPoints(0,4,
              myOriginX                    , (myOriginY + myMajorTickSize) ,
              myOriginX                    ,  myOriginY                    ,
              (myScaleBarWidthInt + myOriginX),  myOriginY                    ,
              (myScaleBarWidthInt + myOriginX), (myOriginY + myMajorTickSize)
              );
      theQPainter->drawPolyline(myTickDownArray);
      //now draw the bar itself in user selected color
      theQPainter->setPen( myForegroundPen );
      myTickDownArray.putPoints(0,4,
              myOriginX                    , (myOriginY + myMajorTickSize) ,
              myOriginX                    ,  myOriginY                    ,
              (myScaleBarWidthInt + myOriginX),  myOriginY                    ,
              (myScaleBarWidthInt + myOriginX), (myOriginY + myMajorTickSize)
              );
      theQPainter->drawPolyline(myTickDownArray);
      break;
    }
    case 1: // tick up
    {
      QPolygon myTickUpArray(4);
      //draw a buffer first so bar shows up on dark images
      theQPainter->setPen( myBackgroundPen );
      myTickUpArray.putPoints(0,4,
              myOriginX                    ,  myOriginY                    ,
              myOriginX                    ,  myOriginY + myMajorTickSize  ,
              (myScaleBarWidthInt + myOriginX),  myOriginY + myMajorTickSize  ,
              (myScaleBarWidthInt + myOriginX),  myOriginY
              );
      theQPainter->drawPolyline(myTickUpArray);
      //now draw the bar itself in user selected color
      theQPainter->setPen( myForegroundPen );
      myTickUpArray.putPoints(0,4,
              myOriginX                    ,  myOriginY                    ,
              myOriginX                    ,  myOriginY + myMajorTickSize  ,
              (myScaleBarWidthInt + myOriginX),  myOriginY + myMajorTickSize  ,
              (myScaleBarWidthInt + myOriginX),  myOriginY
              );
      theQPainter->drawPolyline(myTickUpArray);
      break;
    }
    case 2: // Bar
    {
      QPolygon myBarArray(2);
      //draw a buffer first so bar shows up on dark images
      theQPainter->setPen( myBackgroundPen );
      myBarArray.putPoints(0,2,
              myOriginX                    ,  (myOriginY + (myMajorTickSize/2)),
              (myScaleBarWidthInt + myOriginX),  (myOriginY + (myMajorTickSize/2))
              );
      theQPainter->drawPolyline(myBarArray);
      //now draw the bar itself in user selected color
      theQPainter->setPen( myForegroundPen );
      myBarArray.putPoints(0,2,
              myOriginX                    ,  (myOriginY + (myMajorTickSize/2)),
              (myScaleBarWidthInt + myOriginX),  (myOriginY + (myMajorTickSize/2))
              );
      theQPainter->drawPolyline(myBarArray);
      break;
    }
    case 3: // box
    {
      // Want square corners for a box
      myBackgroundPen.setJoinStyle( Qt::MiterJoin );
      myForegroundPen.setJoinStyle( Qt::MiterJoin );
      QPolygon myBoxArray(5);
      //draw a buffer first so bar shows up on dark images
      theQPainter->setPen( myBackgroundPen );
      myBoxArray.putPoints(0,5,
              myOriginX                    ,  myOriginY,
              (myScaleBarWidthInt + myOriginX),  myOriginY,
              (myScaleBarWidthInt + myOriginX), (myOriginY+myMajorTickSize),
              myOriginX                    , (myOriginY+myMajorTickSize),
              myOriginX                    ,  myOriginY
              );
      theQPainter->drawPolyline(myBoxArray);
      //now draw the bar itself in user selected color
      theQPainter->setPen( myForegroundPen );
      theQPainter->setBrush( QBrush( mColour, Qt::SolidPattern) );
      int midPointX = myScaleBarWidthInt/2 + myOriginX; 
      myBoxArray.putPoints(0,5,
              myOriginX                    ,  myOriginY,
              midPointX,  myOriginY,
              midPointX, (myOriginY+myMajorTickSize),
              myOriginX                    , (myOriginY+myMajorTickSize),
              myOriginX                    ,  myOriginY
              );
      theQPainter->drawPolygon(myBoxArray);

      theQPainter->setBrush( Qt::NoBrush );
      myBoxArray.putPoints(0,5,
              midPointX                    ,  myOriginY,
              (myScaleBarWidthInt + myOriginX),  myOriginY,
              (myScaleBarWidthInt + myOriginX), (myOriginY+myMajorTickSize),
              midPointX                    , (myOriginY+myMajorTickSize),
              midPointX                    ,  myOriginY
              );
      theQPainter->drawPolygon(myBoxArray);
      break;
    }
    default:
      std::cerr << "Unknown style\n";
    }

    //Do actual drawing of scale bar

    //
    //Do drawing of scale bar text
    //

    QColor myBackColor = Qt::white;
    QColor myForeColor = Qt::black;

    //Draw the minimum label buffer
    theQPainter->setPen( myBackColor );
    myFontWidth = myFontMetrics.width( "0" );
    myFontHeight = myFontMetrics.height();

    for (int i = 0-myBufferSize; i <= myBufferSize; i++)
    {
      for (int j = 0-myBufferSize; j <= myBufferSize; j++)
      {
        theQPainter->drawText( int(i +(myOriginX-(myFontWidth/2))),
                              int(j + (myOriginY-(myFontHeight/4))),
                              "0");
      }
    }

    //Draw minimum label
    theQPainter->setPen( myForeColor );

    theQPainter->drawText(
            int(myOriginX-(myFontWidth/2)),
            int(myOriginY-(myFontHeight/4)),
            "0"
            );

    //
    //Draw maximum label
    //
    theQPainter->setPen( myBackColor );
    myFontWidth = myFontMetrics.width( myScaleBarMaxLabel );
    myFontHeight = myFontMetrics.height();
    //first the buffer
    for (int i = 0-myBufferSize; i <= myBufferSize; i++)
    {
      for (int j = 0-myBufferSize; j <= myBufferSize; j++)
      {
        theQPainter->drawText( int(i + (myOriginX+myScaleBarWidthInt-(myFontWidth/2))),
                              int(j + (myOriginY-(myFontHeight/4))),
                              myScaleBarMaxLabel);
      }
    }
    //then the text itself
    theQPainter->setPen( myForeColor );
    theQPainter->drawText(
            int(myOriginX+myScaleBarWidthInt-(myFontWidth/2)),
            int(myOriginY-(myFontHeight/4)),
            myScaleBarMaxLabel
            );

    //
    //Draw unit label
    //
    theQPainter->setPen( myBackColor );
    myFontWidth = myFontMetrics.width( myScaleBarUnitLabel );
    myFontHeight = myFontMetrics.height();
    //first the buffer
    for (int i = 0-myBufferSize; i <= myBufferSize; i++)
    {
      for (int j = 0-myBufferSize; j <= myBufferSize; j++)
      {
        theQPainter->drawText( i + (myOriginX+myScaleBarWidthInt+myTextOffsetX),
                              j + (myOriginY+myMajorTickSize),
                              myScaleBarUnitLabel);
      }
    }
    //then the text itself
    theQPainter->setPen( myForeColor );
    theQPainter->drawText(
            (myOriginX+myScaleBarWidthInt+myTextOffsetX),(myOriginY+myMajorTickSize),
            myScaleBarUnitLabel
            );
  }
}



// Unload the plugin by cleaning up the GUI
void QgsScaleBarPlugin::unload()
{
  // remove the GUI
  qGisInterface->removePluginMenuItem(tr("&Decorations"),menuId);
  qGisInterface->removeToolBarIcon(myQActionPointer);

  // remove the northarrow from the canvas
  disconnect(qGisInterface->getMapCanvas(), SIGNAL(renderComplete(QPainter *)),
	     this, SLOT(renderScaleBar(QPainter *)));
  refreshCanvas();

  delete myQActionPointer;
}

//! set placement of scale bar
void QgsScaleBarPlugin::setPlacement(int placementIndex)
{
  mPlacementIndex = placementIndex;
  QgsProject::instance()->writeEntry("ScaleBar","/Placement",mPlacementIndex);
}

//! set preferred size of scale bar
void QgsScaleBarPlugin::setPreferredSize(int thePreferredSize)
{
  mPreferredSize = thePreferredSize;
  QgsProject::instance()->writeEntry("ScaleBar","/PreferredSize",mPreferredSize);
}

//! set whether the scale bar length should snap to the closes A*10^B
void QgsScaleBarPlugin::setSnapping(bool theSnapping)
{
  mSnapping = theSnapping;
  QgsProject::instance()->writeEntry("ScaleBar","/Snapping",mSnapping);
}

//! set scale bar enable
void QgsScaleBarPlugin::setEnabled(bool theBool)
{
  mEnabled = theBool;
  QgsProject::instance()->writeEntry("ScaleBar","/Enabled",mEnabled);
}
//! set scale bar enable
void QgsScaleBarPlugin::setStyle(int styleIndex)
{
  mStyleIndex = styleIndex;
  QgsProject::instance()->writeEntry("ScaleBar","/Style",mStyleIndex);
}
//! set the scale bar colour
void QgsScaleBarPlugin::setColour(QColor theQColor)
{
  mColour = theQColor;
  QgsProject::instance()->writeEntry("ScaleBar","/ColorRedPart", mColour.red());
  QgsProject::instance()->writeEntry("ScaleBar","/ColorGreenPart", mColour.green());
  QgsProject::instance()->writeEntry("ScaleBar","/ColorBluePart", mColour.blue());
}


/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  return new QgsScaleBarPlugin(theQGisAppPointer, theQgisInterfacePointer);
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return name_;
}

// Return the description
QGISEXTERN QString description()
{
  return description_;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return type_;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return version_;
}

// Delete ourself
QGISEXTERN void unload(QgisPlugin * thePluginPointer)
{
  delete thePluginPointer;
}
