/***************************************************************************
  qgsmapserverexport.cpp - Export QGIS MapCanvas to MapServer
  --------------------------------------
Date                 : 8-Nov-2003
Copyright            : (C) 2003 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsmapserverexport.h"

#include "qgis.h"
#include "qgsdatasourceuri.h"
#include "qgshelpviewer.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsrect.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include <iostream>
#include <fstream>

// constructor
QgsMapserverExport::QgsMapserverExport(QgsMapCanvas * _map, QWidget * parent, Qt::WFlags fl)
  : QDialog(parent, fl), map(_map)
{
  setupUi(this);
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  // These values shouldn't be translated, the units should be in english in the map file
  // Qt designer adds translate() by default
  cmbMapUnits->addItem(QString::fromUtf8("dd"));
  cmbMapUnits->addItem(QString::fromUtf8("feet"));
  cmbMapUnits->addItem(QString::fromUtf8("meters"));
  cmbMapUnits->addItem(QString::fromUtf8("miles"));
  cmbMapUnits->addItem(QString::fromUtf8("inches"));
  cmbMapUnits->addItem(QString::fromUtf8("kilometers"));
}

// Default destructor
QgsMapserverExport::~QgsMapserverExport()
{
}

// Get the base name for the map file
QString QgsMapserverExport::baseName()
{
  QFileInfo fi(txtMapFilePath->text());
  return fi.baseName(true);
}

// Write the map file
bool QgsMapserverExport::write()
{

  //QMessageBox::information(0,"Full Path",fullPath);
  int okToSave = 0;
  // Check for file and prompt for overwrite if it exists
  if (QFile::exists(txtMapFilePath->text()))
  {
    okToSave = QMessageBox::warning(0, tr("Overwrite File?"), txtMapFilePath->text() +
        tr(" exists. \nDo you want to overwrite it?"), tr("Yes"), tr("No"));
  }
  if (okToSave == 0)
  {
    // write the project information to the selected file
    writeMapFile();
    return true;
  } else
  {
    return false;
  }
}


void QgsMapserverExport::setFileName(QString fn)
{
  fullPath = fn;
}

QString QgsMapserverExport::fullPathName()
{
  return fullPath;
}

void QgsMapserverExport::writeMapFile()
{
  // write the map file, making massive assumptions about default values
#ifdef QGISDEBUG
  std::cout << "Opening map file " << txtMapFilePath->text().toLocal8Bit().data() << std::endl;
#endif
  std::ofstream mapFile(txtMapFilePath->text().toLocal8Bit().data());
  if (!mapFile.fail())
  {
    // XXX So, what encoding should we use here???
    mapFile << "# Map file generated by QGIS version " << QGis::qgisVersion << std::endl;
    mapFile << "# Edit this file to customize for your interface" << std::endl;
    mapFile << "# Not all sections are complete. See comments for details." << std::endl;
    if (!chkExpLayersOnly->isChecked())
    {
      // header
      mapFile << "NAME " << txtMapName->text().toLocal8Bit().data() << std::endl;
      mapFile << "STATUS ON" << std::endl;
      mapFile << "\n# Map image size. Change size as desired" << std::endl;
      mapFile << "SIZE " << txtMapWidth->text().toLocal8Bit().data() << " " << txtMapHeight->text().toLocal8Bit().data() << std::endl;
      // web interface definition - this is minimal!
      mapFile << "#" << std::endl;
      mapFile << "# Start of web interface definition. Only the TEMPLATE parameter" << std::endl;
      mapFile << "# must be specified to display a map. See Mapserver documentation" << std::endl;
      mapFile << "#" << std::endl;
      mapFile << "WEB" << std::endl;
      // if no header is supplied, write the header line but comment it out
      if (txtWebHeader->text().isEmpty())
      {
        mapFile << "  # HEADER" << std::endl;
      } else
      {
        // header provided - write it
        mapFile << "  HEADER " << txtWebHeader->text().toLocal8Bit().data() << std::endl;
      }
      // if no template provided, write the template line but comment it out
      if (txtWebTemplate->text().isEmpty())
      {
        mapFile << "  # TEMPLATE" << std::endl;
      } else
      {
        // template provided - write it
        mapFile << "  TEMPLATE " << txtWebTemplate->text().toLocal8Bit().data() << std::endl;
      }
      // if no footer provided, write the footer line but comment it out
      if (txtWebFooter->text().isEmpty())
      {
        mapFile << "  # FOOTER" << std::endl;
      } else
      {
        mapFile << "  FOOTER " << txtWebFooter->text().toLocal8Bit().data() << std::endl;
      }
      QString minScale = txtMinScale->text().isEmpty()?"#MINSCALE":"MINSCALE";
      QString maxScale = txtMinScale->text().isEmpty()?"  #MAXSCALE ":"  MAXSCALE ";
      // write min and maxscale
      mapFile << minScale.toLocal8Bit().data() << txtMinScale->text().toLocal8Bit().data() << std::endl;
      mapFile << maxScale.toLocal8Bit().data() << txtMaxScale->text().toLocal8Bit().data() << std::endl;
      // write comments about the imagepath and image url
      mapFile << "# Set IMAGEPATH to the path where mapserver should\n" <<
        "# write its output\n" <<
        " IMAGEPATH '/tmp/'" << std::endl; 
      mapFile << "# Set IMAGEURL to the url that points to IMAGEPATH" << std::endl; 
      mapFile << " #IMAGEURL '/map_output/'" << std::endl; 
      // end of web section
      mapFile << "END" << std::endl;

      // extent
      mapFile << "\n# Extent based on full extent of QGIS view" << std::endl;
      mapFile << "EXTENT ";
      QgsRect extent = map->extent();
      mapFile << extent.xMin() << " " << extent.yMin() << " ";
      mapFile << extent.xMax() << " " << extent.yMax() << std::endl;
      // units
      mapFile << "UNITS " << cmbMapUnits->currentText().toLocal8Bit().data() << std::endl;
      // image info
      mapFile << "IMAGECOLOR 255 255 255" << std::endl;
      mapFile << "IMAGETYPE " << cmbMapImageType->currentText().toLocal8Bit().data() << std::endl;
      // projection information TODO: support projections :)
      mapFile << "# Projection definition" << std::endl;
      mapFile << "# Projections are not currenlty supported. If desired, add your own" << std::endl;
      mapFile << "# projection information based on Mapserver documentation." << std::endl;
      mapFile << "#" << std::endl;

    } else
    {
      mapFile << " # This file contains layer definitions only and is not a complete" << std::endl;
      mapFile << " # Mapserver map file." << std::endl;
    }

    // write layer definitions 
    for (int i = 0; i < map->layerCount(); i++)
    {
      bool isPolygon = false;
      bool isLine = false;
      QgsMapLayer *lyr = map->getZpos(i);
#ifdef QGISDEBUG
      std::cout << "Mapsrver Export Processing Layer" << std::endl;
#endif
      mapFile << "LAYER" << std::endl;
      QString name = lyr->name().lower();
      // MapServer NAME must be < 20 char and unique
      name.replace(QRegExp(" "), "_");
      name.replace(QRegExp("\\."), "_");
      name.replace(QRegExp("\\("), "_");
      name.replace(QRegExp("\\)"), "_");
      mapFile << "  NAME " << name.toLocal8Bit().data() << std::endl;
      // feature type
#ifdef QGISDEBUG
      std::cout << "\tMapserver Export checking feature type" << std::endl;
#endif
      mapFile << "  TYPE ";
      switch (lyr->featureType())
      {
        case QGis::WKBPoint:
        case QGis::WKBMultiPoint:
          mapFile << "POINT";
          break;
        case QGis::WKBLineString:
        case QGis::WKBMultiLineString:
          mapFile << "LINE";
          isLine = true;
          break;
        case QGis::WKBPolygon:
        case QGis::WKBMultiPolygon:
          mapFile << "POLYGON";
          isPolygon = true;
          break;
                 
      }
      if(lyr->type() == QgsMapLayer::RASTER)
      {
        mapFile << "RASTER";
      }
      mapFile << std::endl;

#ifdef QGISDEBUG
      std::cout << "\tMapserver Export checking visibility" << std::endl;
#endif
      // set visibility (STATUS)
      mapFile << "  STATUS ";
      if (lyr->visible())
      {
        mapFile << "ON";
      } else
      {
        mapFile << "OFF";
      }
      mapFile << std::endl;

      // data source (DATA)
      // Data source spec depends on layer type
#ifdef QGISDEBUG
      std::cout << "\tMapserver Export checking layer type" << std::endl;
#endif
      switch (lyr->type())
      {
        case QgsMapLayer::VECTOR:
          // get the provider type
          {
            QString providerType = 
              dynamic_cast<QgsVectorLayer*>(lyr)->providerType();
            if(providerType == "postgres")
            {
              QgsDataSourceURI *dUri = 
                dynamic_cast<QgsVectorLayer *>(lyr)->getDataProvider()->getURI();
              mapFile << "CONNECTION \"user=" << dUri->username.toLocal8Bit().data();
              if(dUri->password.length() > 0)
              {
                mapFile << " password="<< dUri->password.toLocal8Bit().data();
              }
              mapFile  << " dbname=" << dUri->database.toLocal8Bit().data() 
                << " host=" << dUri->host.toLocal8Bit().data()
                << " port=" << dUri->port.toLocal8Bit().data()
                << "\"" << std::endl; 
              mapFile << "CONNECTIONTYPE postgis" << std::endl; 
              mapFile << "DATA \"" << dUri->geometryColumn.toLocal8Bit().data() << " from " 
                << dUri->table.toLocal8Bit().data() << "\"" << std::endl; 
              if(dUri->sql.length() > 0)
              {
                mapFile << "FILTER \"" << dUri->sql.toLocal8Bit().data() << "\"" << std::endl; 
              }

            }
            else
            {

              // must be an ogr 
              mapFile << "  DATA " << lyr->source().toLocal8Bit().data() << std::endl;
            }
          }
          break;
        case QgsMapLayer::RASTER:
          mapFile << "  DATA " << lyr->source().toLocal8Bit().data() << std::endl; 
          
          break;
      }
#ifdef QGISDEBUG
      std::cout << "\tMapserver Export creating symbol entries" << std::endl;
#endif
      // create a simple class entry based on red fill color and black outline color
      //TODO: adapt the following section to the new symbology

      mapFile << "  CLASS" << std::endl;
      //QListViewItem *li = map->getLegend()->currentItem();
      //    return li->text(0);
#ifdef QGISDEBUG
      std::cout << "\tMapserver Export symbol name" << std::endl;
#endif
      mapFile << "    NAME \"" << lyr->name().toLocal8Bit().data() << "\"" << std::endl;
      mapFile << "    # TEMPLATE" << std::endl;
      if (isPolygon)
      {
#ifdef QGISDEBUG
        std::cout << "\tMapserver Export symbol fill color" << std::endl;
#endif
        // use random fill colors
        // TODO Get fill color from the renderer
        // TODO Figure out what to do for layers that are
        //      rendered with other than the simple symbol
        //      renderer.
        int red = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
        int green = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
        int blue = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));

        mapFile << "    COLOR " << red << " " << green << " " << blue << std::endl;
      }
#ifdef QGISDEBUG
      std::cout << "\tMapserver Export checking for line symbol " << std::endl;
#endif
      if (isPolygon || isLine)
      {
        // use line color
        // TODO Get line color from the renderer
        int red = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
        int green = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
        int blue = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
        mapFile << "    OUTLINECOLOR " << red << " "
          << green << " " << blue << std::endl;
      }
      mapFile << "  END" << std::endl;
      mapFile << "END" << std::endl;
#ifdef QGISDEBUG
      std::cout << "\tMapserver Export layer definition done..." << std::endl;
#endif
    }
    if (!chkExpLayersOnly->isChecked())
    {
      mapFile << "END # Map File";
    }
    mapFile.flush();
    mapFile.close();
  } else
  {
  }
}

void QgsMapserverExport::on_chkExpLayersOnly_clicked()
{
  // disable inputs if only layer objects are being written
  grpMap->setEnabled(!chkExpLayersOnly->isChecked());
  grpWeb->setEnabled(!chkExpLayersOnly->isChecked());
}

void QgsMapserverExport::on_btnChooseFile_clicked()
{
  QString s = QFileDialog::getSaveFileName(
                    this,
                    tr("Choose a filename for the exported map file"),
                    "./",
                    tr("Mapserver files (*.map)" ) );
  txtMapFilePath->setText(s);
}

void QgsMapserverExport::on_buttonHelp_clicked()
{
  //QMessageBox::information(this, "Help","Help");
  QgsHelpViewer *hv = new QgsHelpViewer(this);
  // causes problems in qt3.1.x:  hv->setModal(false);
  hv->setCaption( tr("QGIS Help - Mapserver Export") );
  hv->show();
}
