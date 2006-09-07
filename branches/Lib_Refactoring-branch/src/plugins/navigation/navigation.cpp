/***************************************************************************
  navigation.cpp 
  A simple GPS navigation and routing system

  -------------------
         begin                : [PluginDate]
         copyright            : [(C) Your Name and Date]
         email                : [Your Email]

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id: plugin.cpp 5400 2006-04-30 20:14:08Z wonder $ */

//
// QGIS Specific includes
//

#include <qgisinterface.h>

#include "navigation.h"
#include "navigationgui.h"
#include "gpscore.h"
#include "routingcore.h"

//
// Qt4 Related Includes
//

#include <QToolBar>
#include <QMenuBar>
#include <QMessageBox>
#include <QMenu>
#include <QLineEdit>
#include <QAction>
#include <QApplication>
#include <QCursor>

//non qt includes
#include <iostream>


#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

static const char * const sIdent = "$Id: plugin.cpp 5400 2006-04-30 20:14:08Z wonder $";
static const char * const sName = "Navigation";
static const char * const sDescription = "A simple GPS navigation and routing system";
static const char * const sPluginVersion = "Version 0.1";
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;

//////////////////////////////////////////////////////////////////////
//
// THE FOLLOWING METHODS ARE MANDATORY FOR ALL PLUGINS
//
//////////////////////////////////////////////////////////////////////

/**
 * Constructor for the plugin. The plugin is passed a pointer 
 * an interface object that provides access to exposed functions in QGIS.
 * @param theQGisInterface - Pointer to the QGIS interface object
 */
Navigation::Navigation(QgisInterface * theQgisInterface):
                 QgisPlugin(sName,sDescription,sPluginVersion,sPluginType),
                 mQGisIface(theQgisInterface)
{
}

Navigation::~Navigation()
{

}

/*
 * Initialize the GUI interface for the plugin - this is only called once when the plugin is 
 * added to the plugin registry in the QGIS application.
 */
void Navigation::initGui()
{
  // we create a single menu entry for this plugin - though you can easily add more here if you need to
  QMenu *pluginMenu = mQGisIface->getPluginMenu("&Navigation");
  //set the icon from the resource file
  mMenuId = pluginMenu->insertItem(QIcon(":/navigation/navigation.png"),"&Navigation", this, SLOT(run()));
  //create a tooltip for the menu entry 
  pluginMenu->setWhatsThis(mMenuId, tr("Replace this with a short description of the what the plugin does"));

  // Create the action for tool
  mQActionPointer = new QAction(QIcon(":/navigation/navigation.png"),"Navigation", this);
  // Connect the action to the run
  connect(mQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  // Add the toolbar
  mToolBarPointer = new QToolBar(mQGisIface->getMainWindow(), "Navigation");
  mToolBarPointer->setLabel("Navigation");
  // Add the icon to the toolbar
  mQGisIface->addToolBarIcon(mQActionPointer);

  // create gps core
  mGps = new GpsCore(mQGisIface);
  
  // create routing core
  mRouting = new RoutingCore(mQGisIface);
}
//method defined in interface
void Navigation::help()
{
  //implement me!
}

// Slot called when the menu item is activated
// If you created more menu items / toolbar buttons in initiGui, you should 
// create a separate handler for each action - this single run() method will
// not be enough
void Navigation::run()
{
  NavigationGui *myPluginGui=new NavigationGui(this);
  //listen for when the layer has been made so we can draw it
  connect(myPluginGui, SIGNAL(drawRasterLayer(QString)), this, SLOT(drawRasterLayer(QString)));
  connect(myPluginGui, SIGNAL(drawVectorLayer(QString,QString,QString)), this, SLOT(drawVectorLayer(QString,QString,QString)));
  myPluginGui->show();
}

// Unload the plugin by cleaning up the GUI
void Navigation::unload()
{
  // remove the GUI
  mQGisIface->removePluginMenuItem("&Navigation",mMenuId);
  mQGisIface->removeToolBarIcon(mQActionPointer);
  delete mQActionPointer;
  
  delete mGps;
}

//////////////////////////////////////////////////////////////////////
//
//                  END OF MANDATORY PLUGIN METHODS
//
//////////////////////////////////////////////////////////////////////


QgisInterface* Navigation::qgis()
{
  return mQGisIface;
}

GpsCore* Navigation::gps()
{
  return mGps;
}


//////////////////////////////////////////////////////////////////////////
//
//
//  THE FOLLOWING CODE IS AUTOGENERATED BY THE PLUGIN BUILDER SCRIPT
//    YOU WOULD NORMALLY NOT NEED TO MODIFY THIS, AND YOUR PLUGIN
//      MAY NOT WORK PROPERLY IF YOU MODIFY THIS INCORRECTLY
//
//
//////////////////////////////////////////////////////////////////////////


/** 
 * Required extern functions needed  for every plugin 
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory(QgisInterface * theQgisInterfacePointer)
{
  return new Navigation(theQgisInterfacePointer);
}
// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return sName;
}

// Return the description
QGISEXTERN QString description()
{
  return sDescription;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return sPluginType;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return sPluginVersion;
}

// Delete ourself
QGISEXTERN void unload(QgisPlugin * thePluginPointer)
{
  delete thePluginPointer;
}
