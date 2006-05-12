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
#ifndef QGSCALEBARPLUGIN
#define QGSCALEBARPLUGIN
#include "../qgisplugin.h"

#include "qgisapp.h"
class QToolBar;
class QPainter;
/**
* \class Plugin
* \brief OpenModeller plugin for QGIS
*
*/
class QgsScaleBarPlugin:public QObject, public QgisPlugin
{
  Q_OBJECT public:
      /**
       * Constructor for a plugin. The QgisApp and QgisIface pointers are passed by
       * QGIS when it attempts to instantiate the plugin.
       * @param qgis Pointer to the QgisApp object
       * @param qI Pointer to the QgisIface object.
       */
      QgsScaleBarPlugin(QgisApp * , QgisIface * );


  //! Destructor
  virtual ~ QgsScaleBarPlugin();
  public slots:
  //! init the gui
  virtual void initGui();
  //!set values on the gui when a project is read or the gui first loaded
  void projectRead();
  //!this does the meaty bit of the work
  void renderScaleBar(QPainter *);
  //! Show the dialog box
  void run();
  //! Refresh the map display using the mapcanvas exported via the plugin interface
  void refreshCanvas();
  //! unload the plugin
  void unload();
  //! show the help document
  void help();
  //! set scale bar placement.
  void setPlacement(int);
  //! set preferred size of scale bar
  void setPreferredSize(int);
  //! set whether the scale bar length should snap to the closest A*10^B
  void setSnapping(bool);
  //! set whether scale bar is enabled
  void setEnabled(bool);
  //! set the scale bar style
  void setStyle(int);
  //! set the scale bar colour
  void setColour(QColor);

    private:



  int pluginType;
  //! Id of the plugin's menu. Used for unloading
  int menuId;
  //! Placement of the scale bar. An index and the translated text
  int mPlacementIndex;
  QStringList mPlacementLabels;
  //! The size preferred size of the scale bar
  int mPreferredSize;
  //! Should we snap to integer times power of 10?
  bool mSnapping;
  //! Scale bar enabled?
  bool mEnabled;
  //! Style of scale bar. An index and the translated text
  int mStyleIndex;
  QStringList mStyleLabels;
  //! The scale bar colour
  QColor mColour;

  //! Pointer to our toolbar
  QToolBar *toolBarPointer;
  //! Pionter to QGIS main application object
  QgisApp *qgisMainWindowPointer;
  //! Pointer to the QGIS interface object
  QgisIface *qGisInterface;
  //! Pointer to the QAction object used in the menu and toolbar
  QAction *myQActionPointer;
};

#endif
