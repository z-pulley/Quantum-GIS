/***************************************************************************
  qgsvectorlayer.cpp 
  This class implements a generic means to display vector layers. The features
  and attributes are read from the data store using a "data provider" plugin.
  QgsVectorLayer can be used with any data store for which an appropriate 
  plugin is available. 
  -------------------
begin                : Oct 29, 2003
copyright            : (C) 2003 by Gary E.Sherman
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
/*  $Id$ */

#include <iostream>
#include <cfloat>
#include <cstring>
#include <sstream>
#include <memory>

#include <qapplication.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qpointarray.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qlibrary.h>
#include <qpicture.h>
#include <qsettings.h>

#include "qgisapp.h"
#include "qgsrect.h"
#include "qgspoint.h"
#include "qgscoordinatetransform.h"
#include "qgsvectorlayer.h"
#include "qgsidentifyresults.h"
#include "qgsattributetable.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgslegenditem.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgsrenderer.h"
#include "qgssinglesymrenderer.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgscontinuouscolrenderer.h"
#include "qgsgraduatedmarenderer.h"
#include "qgssimarenderer.h"
#include "qgsrenderitem.h"
#include "qgssisydialog.h"
#include "qgsproviderregistry.h"
#include "qgsrect.h"
#include "qgslabelattributes.h"
#include "qgslabel.h"

//#include "wkbheader.h"

#ifdef TESTPROVIDERLIB
#include <dlfcn.h>
#endif


static const char * const ident_ = "$Id$";


// typedef for the QgsDataProvider class factory
typedef QgsDataProvider * create_it(const char * uri);



QgsVectorLayer::QgsVectorLayer(QString vectorLayerPath, 
                               QString baseName, 
                               QString providerKey)
    : QgsMapLayer(VECTOR, baseName, vectorLayerPath), 
      providerKey(providerKey), 
      tabledisplay(0), 
      m_renderer(0), 
      m_propertiesDialog(0), 
      m_rendererDialog(0)
{
  // initialize the identify results pointer
  ir = 0;

#ifdef QGISDEBUG
  std::cerr << "VECTORLAYERPATH: " << vectorLayerPath.ascii() << std::endl;
  std::cerr << "BASENAME: " << baseName.ascii() << std::endl;
#endif

  // if we're given a provider type, try to create and bind one to this layer
  if ( ! providerKey.isEmpty() )
  {
      setDataProvider( providerKey );
  }
  
  //TODO - fix selection code that formerly used
  //       a boolean vector and set every entry to false

  //draw the selected features in yellow
  selectionColor.setRgb(255, 255, 0);

  // Default for the popup menu
  popMenu = 0;
  
  // Get the update threshold from user settings. We
  // do this only on construction to avoid the penality of
  // fetching this each time the layer is drawn. If the user
  // changes the threshold from the preferences dialog, it will
  // have no effect on existing layers
  QSettings settings;
  updateThreshold = settings.readNumEntry("qgis/map/updateThreshold", 1000);
} // QgsVectorLayer ctor



QgsVectorLayer::~QgsVectorLayer()
{
#ifdef QGISDEBUG
  std::cerr << "In QgsVectorLayer destructor" << std::endl;
#endif
  if (tabledisplay)
  {
    tabledisplay->close();
    delete tabledisplay;
  }
  if (m_renderer)
  {
    delete m_renderer;
  }
  if (m_rendererDialog)
  {
    delete m_rendererDialog;
  }
  if (m_propertiesDialog)
  {
    delete m_propertiesDialog;
  }
  // delete the provider object
  delete dataProvider;
  // delete the popu pmenu
  delete popMenu;
  // delete the provider lib pointer
  delete myLib;
}

QString QgsVectorLayer::providerType()
{
  return providerKey;
}

/** 
 * sets the preferred display field based on some fuzzy logic
 */
void QgsVectorLayer::setDisplayField(QString fldName)
{
  // If fldName is provided, use it as the display field, otherwise
  // determine the field index for the feature column of the identify
  // dialog. We look for fields containing "name" first and second for
  // fields containing "id". If neither are found, the first field
  // is used as the node.
  QString idxName="";
  QString idxId="";

  std::vector < QgsField > fields = dataProvider->fields();
  if(!fldName.isEmpty())
  {
    // find the index for this field
    fieldIndex = fldName;
    /*
    for(int i = 0; i < fields.size(); i++)
    {
      if(QString(fields[i].name()) == fldName)
      {
        fieldIndex = i;
        break;
      }
    }
    */
  }else{
  int j = 0;
  for (int j = 0; j < fields.size(); j++)
  {

    QString fldName = fields[j].name();
#ifdef QGISDEBUG
    std::cerr << "Checking field " << fldName << std::endl;
#endif
    // Check the fields and keep the first one that matches.
    // We assume that the user has organized the data with the
    // more "interesting" field names first. As such, name should
    // be selected before oldname, othername, etc.
    if (fldName.find("name", false) > -1)
    {
      if(idxName.isEmpty())
      {
        idxName = fldName;
      }
    }
    if (fldName.find("descrip", false) > -1)
    {
      if(idxName.isEmpty())
      {
        idxName = fldName;
      }
    }
    if (fldName.find("id", false) > -1)
    {
      if(idxId.isEmpty())
      {
        idxId = fldName;
      }
    }
  }

  //if there were no fields in the dbf just return - otherwise qgis segfaults!
  if (fields.size() == 0) return;

  if (idxName.length() > 0)
  {
    fieldIndex = idxName;
  } else
  {
    if (idxId.length() > 0)
    {
      fieldIndex = idxId;
    } else
    {
      fieldIndex = fields[0].name();
    }
  }

    // set this to be the label field as well
    setLabelField(fieldIndex);
  }
}

void QgsVectorLayer::draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf, QPaintDevice* dst)
{
  if ( /*1 == 1 */ m_renderer)
  {
    // painter is active (begin has been called
    /* Steps to draw the layer
       1. get the features in the view extent by SQL query
       2. read WKB for a feature
       3. transform
       4. draw
       */

    QPen pen;
    /*Pointer to a marker image*/
    QPicture marker;
    /*Scale factor of the marker image*/
    double markerScaleFactor=1;

    // select the records in the extent. The provider sets a spatial filter
    // and sets up the selection set for retrieval
#ifdef QGISDEBUG
    qWarning("Selecting features based on view extent");
#endif
    dataProvider->reset();
    dataProvider->select(viewExtent);
    int featureCount = 0;
    //  QgsFeature *ftest = dataProvider->getFirstFeature();
#ifdef QGISDEBUG
    qWarning("Starting draw of features");
#endif
    QgsFeature *fet;
    unsigned char *feature;
    bool attributesneeded = m_renderer->needsAttributes();

    double *x;
    double *y;
    int *nPoints;
    int *numRings;
    int *numPolygons;
    int numPoints;
    int numLineStrings;
    int idx, jdx, kdx;
    unsigned char *ptr;
    char lsb;
    QgsPoint pt;
    QPointArray *pa;
    int wkbType;
    
    std::list<int> attributes=m_renderer->classificationAttributes();
    if ( mLabelOn ) { // Add fields required for labels
	mLabel->addRequiredFields ( &attributes );
    }
    
    while((fet = dataProvider->getNextFeature(attributes)))
    {
      // If update threshold is greater than 0, check to see if
      // the threshold has been exceeded
      if(updateThreshold > 0)
      {
        //copy the drawing buffer every updateThreshold elements
        if(featureCount%updateThreshold==0)
        {
          bitBlt(dst,0,0,p->device(),0,0,-1,-1,Qt::CopyROP,false);
        }
      }

      if (fet == 0)
      {
#ifdef QGISDEBUG
        std::cerr << "get next feature returned null\n";
#endif
      } else
      {
	  bool sel=selected.find(fet->featureId()) != selected.end();
	  m_renderer->renderFeature(p, fet, &marker, &markerScaleFactor, sel); 

        // get the wkb representation
        feature = fet->getGeometry();
        //  if (feature != 0) {
        //    std::cout << featureCount << "'the feature is null\n";
        //TODO - add this to the debug options -- if we decide to keep it
        //wkbHeader header;
        //memcpy((void *)&header,feature,sizeof(header));
        //std::cout << "Endian:" << header.endian << " WkbType:" << header.wkbType << std::endl; 

        // FIX for the endian problem on osx (possibly sparc?)
        // TODO Restructure this whole wkb reading code to use
        // wkb structures as defined at (among other places):
        // http://publib.boulder.ibm.com/infocenter/db2help/index.jsp?topic=/com.ibm.db2.udb.doc/opt/rsbp4121.htm
        memcpy(&wkbType, (feature+1), sizeof(wkbType));

        //  std::cout << "Feature type: " << wkbType << std::endl;
        // read each feature based on its type

        switch (wkbType)
        {
          case WKBPoint:

            //  fldDef = fet->GetFieldDefnRef(1);
            //   fld = fldDef->GetNameRef();
            //NEEDTHIS?  val = fet->GetFieldAsString(1);
            //std::cout << val << "\n";

            x = (double *) (feature + 5);
            y = (double *) (feature + 5 + sizeof(double));
            //    std::cout << "transforming point\n";
            pt.setX(*x);
            pt.setY(*y);
            cXf->transform(&pt);
            //std::cout << "drawing marker for feature " << featureCount << "\n";
            p->drawRect(static_cast<int>(pt.x()), static_cast<int>(pt.y()), 5, 5);
            p->scale(markerScaleFactor,markerScaleFactor);
            p->drawPicture((int)(static_cast<int>(pt.x()) / markerScaleFactor - marker.boundingRect().width() / 2), 
                           (int)(static_cast<int>(pt.y()) / markerScaleFactor - marker.boundingRect().height() / 2),
                           marker);
            p->resetXForm(); 

            break;

          case WKBLineString:

            // get number of points in the line
            ptr = feature + 5;
            nPoints = (int *) ptr;
            ptr = feature + 1 + 2 * sizeof(int);
            for (idx = 0; idx < *nPoints; idx++)
            {
              x = (double *) ptr;
              ptr += sizeof(double);
              y = (double *) ptr;
              ptr += sizeof(double);
              // transform the point
              pt.setX(*x);
              pt.setY(*y);
              cXf->transform(&pt);
              if (idx == 0)
                p->moveTo(static_cast<int>(pt.x()), static_cast<int>(pt.y()));
              else
                p->lineTo(static_cast<int>(pt.x()), static_cast<int>(pt.y()));
            }
            break;

          case WKBMultiLineString:

            numLineStrings = (int) (feature[5]);
            ptr = feature + 9;
            for (jdx = 0; jdx < numLineStrings; jdx++)
            {
              // each of these is a wbklinestring so must handle as such
              lsb = *ptr;
              ptr += 5; // skip type since we know its 2
              nPoints = (int *) ptr;
              ptr += sizeof(int);
              for (idx = 0; idx < *nPoints; idx++)
              {
                x = (double *) ptr;
                ptr += sizeof(double);
                y = (double *) ptr;
                ptr += sizeof(double);
                // transform the point
                pt.setX(*x);
                pt.setY(*y);
                cXf->transform(&pt);
                if (idx == 0)
                  p->moveTo(static_cast<int>(pt.x()), static_cast<int>(pt.y()));
                else
                  p->lineTo(static_cast<int>(pt.x()), static_cast<int>(pt.y()));
              }
            }
            break;

          case WKBPolygon:

            // get number of rings in the polygon
            numRings = (int *) (feature + 1 + sizeof(int));

            int *ringStart; // index of first point for each ring
            int *ringNumPoints; // number of points in each ring
            ringStart = new int[*numRings];
            ringNumPoints = new int[*numRings];

            int x0, y0, pdx;
            pdx = 0;
            ptr = feature + 1 + 2 * sizeof(int); // set pointer to the first ring
            for (idx = 0; idx < *numRings; idx++) {
              // get number of points in the ring
              nPoints = (int *) ptr;
              ringStart[idx] = pdx;
              ringNumPoints[idx] = *nPoints;
              ptr += 4;
              if ( idx == 0 ) { 
                pa = new QPointArray(*nPoints);
              } else {
                pa->resize ( pa->size() + *nPoints + 1 ); // better to calc size for all rings before?
              }
              for (jdx = 0; jdx < *nPoints; jdx++) {
                // add points to a point array for drawing the polygon
                x = (double *) ptr;
                ptr += sizeof(double);
                y = (double *) ptr;
                ptr += sizeof(double);
                pt.setX(*x);
                pt.setY(*y);
                cXf->transform(&pt);
                pa->setPoint(pdx++, static_cast<int>(pt.x()), static_cast<int>(pt.y()));
              }
              if ( idx == 0 ) { // remember last outer ring point
                x0 = static_cast<int>(pt.x());
                y0 = static_cast<int>(pt.y());
              } else { // return to x0,y0 (inner rings - islands)
                pa->setPoint(pdx++, x0, y0);
              }
            }
            // draw the polygon fill
            pen = p->pen(); // store current pen
            p->setPen ( Qt::NoPen ); // no boundary
            p->drawPolygon(*pa);

            // draw outline
            p->setPen ( pen );
            p->setBrush ( Qt::NoBrush );
            for (idx = 0; idx < *numRings; idx++) {
              p->drawPolygon( *pa, FALSE, ringStart[idx], ringNumPoints[idx]);
            }

            delete pa;
            delete ringStart;
            delete ringNumPoints;

            break;

          case WKBMultiPolygon:

            // get the number of polygons
            ptr = feature + 5;
            numPolygons = (int *) ptr;
            ptr = feature + 9;
            for (kdx = 0; kdx < *numPolygons; kdx++)
            {
              //skip the endian and feature type info and
              // get number of rings in the polygon
              ptr+=5;
              numRings = (int *) ptr;
              ptr += 4;
              for (idx = 0; idx < *numRings; idx++)
              {
                // get number of points in the ring
                nPoints = (int *) ptr;
                ptr += 4;
                pa = new QPointArray(*nPoints);
                for (jdx = 0; jdx < *nPoints; jdx++)
                {
                  // add points to a point array for drawing the polygon
                  x = (double *) ptr;
                  ptr += sizeof(double);
                  y = (double *) ptr;
                  ptr += sizeof(double);
                  // std::cout << "Transforming " << *x << "," << *y << " to ";

                  pt.setX(*x);
                  pt.setY(*y);
                  cXf->transform(&pt);
                  //std::cout << pt.xToInt() << "," << pt.yToInt() << std::endl;
                  pa->setPoint(jdx, static_cast<int>(pt.x()), static_cast<int>(pt.y()));

                }
                // draw the ring
                p->drawPolygon(*pa);
                delete pa;
              }
            }
            break;

          default:
#ifdef QGISDEBUG
            std::cout << "UNKNOWN WKBTYPE ENCOUNTERED\n";
#endif
            break;
        }

	// Render labels
	if ( mLabelOn ) {
	    mLabel->renderLabel ( p, viewExtent, cXf, dst, fet, sel);
	}

	delete fet;

        //std::cout << "deleting feature[]\n";
        //      std::cout << geom->getGeometryName() << std::endl;
        featureCount++;
      }
      //qApp->processEvents();
      }
#ifdef QGISDEBUG
      std::cerr << "Total features processed is " << featureCount << std::endl;
#endif
      qApp->processEvents();
    } else
    {
#ifdef QGISDEBUG
      qWarning("Warning, QgsRenderer is null in QgsVectorLayer::draw()");
#endif
    }
  }

  int QgsVectorLayer::endian()
  {
    char *chkEndian = new char[4];
    memset(chkEndian, '\0', 4);
    chkEndian[0] = 0xE8;

    int *ce = (int *) chkEndian;
    int retVal;
    if (232 == *ce)
      retVal = NDR;
    else
      retVal = XDR;
    delete[]chkEndian;
    return retVal;
  }

  void QgsVectorLayer::identify(QgsRect * r)
  {
    QApplication::setOverrideCursor(Qt::waitCursor);
    dataProvider->select(r, true);
    int featureCount = 0;
    QgsFeature *fet;
    unsigned char *feature;
    // display features falling within the search radius
    if(ir){
      delete ir;
    }
    ir = 0;
    while ((fet = dataProvider->getNextFeature(true)))
    {
      featureCount++;
      if (featureCount == 1)
      {
        ir = new QgsIdentifyResults();
      }

      QListViewItem *featureNode = ir->addNode("foo");
      featureNode->setText(0, fieldIndex);
      std::vector < QgsFeatureAttribute > attr = fet->attributeMap();
      for (int i = 0; i < attr.size(); i++)
      {
#ifdef QGISDEBUG
        std::cout << attr[i].fieldName() << " == " << fieldIndex << std::endl;
#endif
        if (attr[i].fieldName().lower() == fieldIndex)
        {
          featureNode->setText(1, attr[i].fieldValue());
        }
        ir->addAttribute(featureNode, attr[i].fieldName(), attr[i].fieldValue());
      }
      delete fet;
    }

#ifdef QGISDEBUG
    std::cout << "Feature count on identify: " << featureCount << std::endl;
#endif
    if (ir)
    {
      ir->setTitle(name());
      // restore the identify window position and show it
      ir->restorePosition();
    }
    QApplication::restoreOverrideCursor();
    if (featureCount == 0)
    {
      QMessageBox::information(0, tr("No features found"), tr("No features were found in the active layer at the point you clicked"));
    }

  }
  void QgsVectorLayer::table()
  {
    if (tabledisplay)
    {
      tabledisplay->raise();
    } else
    {
      // display the attribute table
      QApplication::setOverrideCursor(Qt::waitCursor);
      dataProvider->reset();
      int numFields = dataProvider->fieldCount();
      tabledisplay = new QgsAttributeTableDisplay();
QObject:connect(tabledisplay, SIGNAL(deleted()), this, SLOT(invalidateTableDisplay()));
        tabledisplay->table()->setNumRows(dataProvider->featureCount());
        tabledisplay->table()->setNumCols(numFields + 1); //+1 for the id-column

        int row = 0;
        // set up the column headers
        QHeader *colHeader = tabledisplay->table()->horizontalHeader();
        colHeader->setLabel(0, "id"); //label for the id-column
        std::vector < QgsField > fields = dataProvider->fields();
        //for (int h = 0; h < numFields; h++) {
        for (int h = 1; h <= numFields; h++)
        {
          colHeader->setLabel(h, fields[h - 1].name());
        }
        QgsFeature *fet;
        while ((fet = dataProvider->getNextFeature(true)))
        {

          //id-field
          tabledisplay->table()->setText(row, 0, QString::number(fet->featureId()));
          tabledisplay->table()->insertFeatureId(fet->featureId(), row);  //insert the id into the search tree of qgsattributetable
          std::vector < QgsFeatureAttribute > attr = fet->attributeMap();
          for (int i = 0; i < attr.size(); i++)
          {
            // get the field values
            tabledisplay->table()->setText(row, i + 1, attr[i].fieldValue());
          }
          row++;
          delete fet;
        }

        // reset the pointer to start of fetabledisplayures so
        // subsequent reads will not fail
        dataProvider->reset();
        tabledisplay->table()->setSorting(true);


        tabledisplay->setTitle(tr("Attribute table - ") + name());
        tabledisplay->show();
        tabledisplay->table()->clearSelection();  //deselect the first row


        QObject::disconnect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));

        for (std::map < int, bool >::iterator it = selected.begin(); it != selected.end(); ++it)
        {
          tabledisplay->table()->selectRowWithId(it->first);
#ifdef QGISDEBUG
          qWarning("selecting row with id " + QString::number(it->first));
#endif
        }

        QObject::connect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));

        //etablish the necessary connections between the table and the shapefilelayer
        QObject::connect(tabledisplay->table(), SIGNAL(selected(int)), this, SLOT(select(int)));
        QObject::connect(tabledisplay->table(), SIGNAL(selectionRemoved()), this, SLOT(removeSelection()));
        QObject::connect(tabledisplay->table(), SIGNAL(repaintRequested()), this, SLOT(triggerRepaint()));
        QApplication::restoreOverrideCursor();
    }

    }

    void QgsVectorLayer::select(int number)
    {
      selected[number] = true;
    }

    void QgsVectorLayer::select(QgsRect * rect, bool lock)
    {
      QApplication::setOverrideCursor(Qt::waitCursor);
      // normalize the rectangle
      rect->normalize();
      if (tabledisplay)
      {
        QObject::disconnect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));
        QObject::disconnect(tabledisplay->table(), SIGNAL(selected(int)), this, SLOT(select(int))); //disconnecting because of performance reason
      }

      if (lock == false)
      {
        removeSelection();        //only if ctrl-button is not pressed
        if (tabledisplay)
        {
          tabledisplay->table()->clearSelection();
        }
      }

      dataProvider->select(rect, true);

      QgsFeature *fet;

      while (fet = dataProvider->getNextFeature(true))
      {
        select(fet->featureId());
        if (tabledisplay)
        {
          tabledisplay->table()->selectRowWithId(fet->featureId());
        }
      }

      if (tabledisplay)
      {
        QObject::connect(tabledisplay->table(), SIGNAL(selectionChanged()), tabledisplay->table(), SLOT(handleChangedSelections()));
        QObject::connect(tabledisplay->table(), SIGNAL(selected(int)), this, SLOT(select(int)));  //disconnecting because of performance reason
      }
      triggerRepaint();
      QApplication::restoreOverrideCursor();
    }

    void QgsVectorLayer::removeSelection()
    {
      selected.clear();
    }

    void QgsVectorLayer::triggerRepaint()
    {
      emit repaintRequested();
    }

    void QgsVectorLayer::invalidateTableDisplay()
    {
      tabledisplay = 0;
    }

    QgsDataProvider *QgsVectorLayer::getDataProvider()
    {
      return dataProvider;
    }

    void QgsVectorLayer::showLayerProperties()
    {
      if (m_propertiesDialog)
      {
	m_propertiesDialog->reset();
        m_propertiesDialog->raise();
        m_propertiesDialog->show();
      } else
      {
        m_propertiesDialog = new QgsDlgVectorLayerProperties(this);
	m_propertiesDialog->reset();
        m_propertiesDialog->show();
      }
    }

    QgsRenderer *QgsVectorLayer::renderer()
    {
      return m_renderer;
    }

    QDialog *QgsVectorLayer::rendererDialog()
    {
      return m_rendererDialog;
    }

    void QgsVectorLayer::setRenderer(QgsRenderer * r)
    {
      if (r != m_renderer)
      {
        if (m_renderer)           //delete any previous renderer
        {
          delete m_renderer;
        }

        m_renderer = r;
      }
    }

    void QgsVectorLayer::setRendererDialog(QDialog * dialog)
    {
      if (dialog != m_rendererDialog)
      {
        if (m_rendererDialog)
        {
          delete m_rendererDialog;
        }
        m_rendererDialog = dialog;
      }
    }

    QGis::VectorType QgsVectorLayer::vectorType()
    {
      if (dataProvider)
      {
        int type = dataProvider->geometryType();
        switch (type)
        {
          case QGis::WKBPoint:
            return QGis::Point;
          case QGis::WKBLineString:
            return QGis::Line;
          case QGis::WKBPolygon:
            return QGis::Polygon;
          case QGis::WKBMultiPoint:
            return QGis::Point;
          case QGis::WKBMultiLineString:
            return QGis::Line;
          case QGis::WKBMultiPolygon:
            return QGis::Polygon;
        }
      } else
      {
#ifdef QGISDEBUG
        qWarning("warning, pointer to dataProvider is null in QgsVectorLayer::vectorType()");
#endif
      }
    }

    QgsDlgVectorLayerProperties *QgsVectorLayer::propertiesDialog()
    {
      return m_propertiesDialog;
    }

    void QgsVectorLayer::initContextMenu(QgisApp * app)
    {
      popMenu = new QPopupMenu();
      QLabel *myPopupLabel = new QLabel( popMenu );
      myPopupLabel->setFrameStyle( QFrame::Panel | QFrame::Raised );
      myPopupLabel->setText( tr("<center><b>Vector Layer</b></center>") );
      popMenu->insertItem(myPopupLabel,0);
      popMenu->insertItem(tr("&Zoom to extent of selected layer"), app, SLOT(zoomToLayerExtent()));
      popMenu->insertItem(tr("&Open attribute table"), app, SLOT(attributeTable()));
      popMenu->insertSeparator();
      popMenu->insertItem(tr("&Properties"), this, SLOT(showLayerProperties()));
      //show in overview slot is implemented in maplayer superclass!
      mShowInOverviewItemId = popMenu->insertItem(tr("Show In &Overview"), this, SLOT(toggleShowInOverview()));

      popMenu->insertSeparator();
      popMenu->insertItem(tr("&Remove"), app, SLOT(removeLayer()));

    }

    //
    QPopupMenu *QgsVectorLayer::contextMenu()
    {
      return popMenu;
    }

    QgsRect QgsVectorLayer::bBoxOfSelected()
    {
      QgsRect rect(DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX);
      dataProvider->reset();

      QgsFeature *fet;
      unsigned char *feature;

      double *x;
      double *y;
      int *nPoints;
      int *numRings;
      int *numPolygons;
      int numPoints;
      int numLineStrings;
      int idx, jdx, kdx;
      unsigned char *ptr;
      char lsb;
      QgsPoint pt;
      QPointArray *pa;
      int wkbType;

      while ((fet = dataProvider->getNextFeature(false)))
      {
        if (selected.find(fet->featureId()) != selected.end())
        {
          feature = fet->getGeometry();
          wkbType = (int) feature[1];


          switch (wkbType)
          {
            case WKBPoint:
              x = (double *) (feature + 5);
              y = (double *) (feature + 5 + sizeof(double));
              if (*x < rect.xMin())
              {
                rect.setXmin(*x);
              }
              if (*x > rect.xMax())
              {
                rect.setXmax(*x);
              }
              if (*y < rect.yMin())
              {
                rect.setYmin(*y);
              }
              if (*y > rect.yMax())
              {
                rect.setYmax(*y);
              }
              break;

            case WKBLineString:
              // get number of points in the line
              ptr = feature + 5;
              nPoints = (int *) ptr;
              ptr = feature + 1 + 2 * sizeof(int);
              for (idx = 0; idx < *nPoints; idx++)
              {
                x = (double *) ptr;
                ptr += sizeof(double);
                y = (double *) ptr;
                ptr += sizeof(double);
                if (*x < rect.xMin())
                {
                  rect.setXmin(*x);
                }
                if (*x > rect.xMax())
                {
                  rect.setXmax(*x);
                }
                if (*y < rect.yMin())
                {
                  rect.setYmin(*y);
                }
                if (*y > rect.yMax())
                {
                  rect.setYmax(*y);
                }
              }
              break;

            case WKBMultiLineString:
              numLineStrings = (int) (feature[5]);
              ptr = feature + 9;
              for (jdx = 0; jdx < numLineStrings; jdx++)
              {
                // each of these is a wbklinestring so must handle as such
                lsb = *ptr;
                ptr += 5;   // skip type since we know its 2
                nPoints = (int *) ptr;
                ptr += sizeof(int);
                for (idx = 0; idx < *nPoints; idx++)
                {
                  x = (double *) ptr;
                  ptr += sizeof(double);
                  y = (double *) ptr;
                  ptr += sizeof(double);
                  if (*x < rect.xMin())
                  {
                    rect.setXmin(*x);
                  }
                  if (*x > rect.xMax())
                  {
                    rect.setXmax(*x);
                  }
                  if (*y < rect.yMin())
                  {
                    rect.setYmin(*y);
                  }
                  if (*y > rect.yMax())
                  {
                    rect.setYmax(*y);
                  }
                }
              }
              break;

            case WKBPolygon:
              // get number of rings in the polygon
              numRings = (int *) (feature + 1 + sizeof(int));
              ptr = feature + 1 + 2 * sizeof(int);
              for (idx = 0; idx < *numRings; idx++)
              {
                // get number of points in the ring
                nPoints = (int *) ptr;
                ptr += 4;
                for (jdx = 0; jdx < *nPoints; jdx++)
                {
                  // add points to a point array for drawing the polygon
                  x = (double *) ptr;
                  ptr += sizeof(double);
                  y = (double *) ptr;
                  ptr += sizeof(double);
                  if (*x < rect.xMin())
                  {
                    rect.setXmin(*x);
                  }
                  if (*x > rect.xMax())
                  {
                    rect.setXmax(*x);
                  }
                  if (*y < rect.yMin())
                  {
                    rect.setYmin(*y);
                  }
                  if (*y > rect.yMax())
                  {
                    rect.setYmax(*y);
                  }
                }
              }
              break;

            case WKBMultiPolygon:
              // get the number of polygons
              ptr = feature + 5;
              numPolygons = (int *) ptr;
              for (kdx = 0; kdx < *numPolygons; kdx++)
              {
                //skip the endian and feature type info and
                // get number of rings in the polygon
                ptr = feature + 14;
                numRings = (int *) ptr;
                ptr += 4;
                for (idx = 0; idx < *numRings; idx++)
                {
                  // get number of points in the ring
                  nPoints = (int *) ptr;
                  ptr += 4;
                  for (jdx = 0; jdx < *nPoints; jdx++)
                  {
                    // add points to a point array for drawing the polygon
                    x = (double *) ptr;
                    ptr += sizeof(double);
                    y = (double *) ptr;
                    ptr += sizeof(double);
                    if (*x < rect.xMin())
                    {
                      rect.setXmin(*x);
                    }
                    if (*x > rect.xMax())
                    {
                      rect.setXmax(*x);
                    }
                    if (*y < rect.yMin())
                    {
                      rect.setYmin(*y);
                    }
                    if (*y > rect.yMax())
                    {
                      rect.setYmax(*y);
                    }
                  }
                }
              }
              break;

            default:
#ifdef QGISDEBUG
              std::cout << "UNKNOWN WKBTYPE ENCOUNTERED\n";
#endif
              break;

          }
          delete[]feature;
        }
      }
      return rect;
    }

    void QgsVectorLayer::setLayerProperties(QgsDlgVectorLayerProperties * properties)
    {
      if (m_propertiesDialog)
      {
        delete m_propertiesDialog;
      }
      m_propertiesDialog = properties;
    }



    QgsFeature * QgsVectorLayer::getFirstFeature(bool fetchAttributes) const
    {
      if ( ! dataProvider )
      {
        std::cerr << __FILE__ << ":" << __LINE__ 
          << " QgsVectorLayer::getFirstFeature() invoked with null dataProvider\n";
        return 0x0;
      }

      return dataProvider->getFirstFeature( fetchAttributes );
    } // QgsVectorLayer::getFirstFeature


    QgsFeature * QgsVectorLayer::getNextFeature(bool fetchAttributes) const
    {
      if ( ! dataProvider )
      {
        std::cerr << __FILE__ << ":" << __LINE__ 
          << " QgsVectorLayer::getNextFeature() invoked with null dataProvider\n";
        return 0x0;
      }

      return dataProvider->getNextFeature( fetchAttributes );
    } // QgsVectorLayer::getNextFeature



    bool QgsVectorLayer::getNextFeature(QgsFeature &feature, bool fetchAttributes) const
    {
      if ( ! dataProvider )
      {
        std::cerr << __FILE__ << ":" << __LINE__ 
          << " QgsVectorLayer::getNextFeature() invoked with null dataProvider\n";
        return false;
      }

      return dataProvider->getNextFeature( feature, fetchAttributes );
    } // QgsVectorLayer::getNextFeature



    long QgsVectorLayer::featureCount() const
    {
      if ( ! dataProvider )
      {
        std::cerr << __FILE__ << ":" << __LINE__ 
          << " QgsVectorLayer::featureCount() invoked with null dataProvider\n";
        return 0;
      }

      return dataProvider->featureCount();
    } // QgsVectorLayer::featureCount


    int QgsVectorLayer::fieldCount() const
    {
      if ( ! dataProvider )
      {
        std::cerr << __FILE__ << ":" << __LINE__ 
          << " QgsVectorLayer::fieldCount() invoked with null dataProvider\n";
        return 0;
      }

      return dataProvider->fieldCount();
    } // QgsVectorLayer::fieldCount


    std::vector<QgsField> const& QgsVectorLayer::fields() const
    {
      if ( ! dataProvider )
      {
        std::cerr << __FILE__ << ":" << __LINE__ 
          << " QgsVectorLayer::fields() invoked with null dataProvider\n";

        static std::vector<QgsField> bogus; // empty, bogus container
        return bogus;
      }

      return dataProvider->fields();
    } // QgsVectorLayer::fields()


bool QgsVectorLayer::addFeature(QgsFeature* f)
{
    if(dataProvider)
    {
	int end=endian();
	memcpy(f->getGeometry(),&end,1);
	if(dataProvider->addFeature(f))
	{
	    if (tabledisplay)
	    {
		tabledisplay->close();
		delete tabledisplay;
		tabledisplay=0;
	    }
	    return true;
	}
    }
    return false;
}

bool QgsVectorLayer::deleteSelectedFeatures()
{
#ifdef QGISDEBUG
    qWarning("entering QgsVectorLayer::deleteSelectedFeatures");
#endif 
    bool resvalue=true;
    for(std::map<int,bool>::iterator it=selected.begin();it!=selected.end();++it)
    {
	if(it->second==true)
	{
#ifdef QGISDEBUG
    qWarning("selected feature detected");
#endif	    
	    if(!dataProvider->deleteFeature(it->first))
	    {
		resvalue=false;
	    }
	}
    }
    triggerRepaint();
    return resvalue;
}

QgsLabel * QgsVectorLayer::label()
{
    return mLabel;
}

void QgsVectorLayer::setLabelOn ( bool on )
{
    mLabelOn = on;
}

bool QgsVectorLayer::labelOn ( void )
{
    return mLabelOn;
}



bool QgsVectorLayer::readXML_( QDomNode & layer_node )
{
    //process provider key
    QDomNode pkeyNode = layer_node.namedItem("provider");

    if (pkeyNode.isNull())
    {
        providerKey = "";
    }
    else
    {
        QDomElement pkeyElt = pkeyNode.toElement();
        providerKey = pkeyElt.text();
    }
	
    // determine type of vector layer
    if ( ! providerKey.isNull() )
    {
        // if the provider string isn't empty, then we successfully
        // got the stored provider
    } else if ((dataSource.find("host=") > -1) && 
               (dataSource.find("dbname=") > -1))
    {
        providerKey = "postgres";
    } else
    {
        providerKey = "ogr";
    }

    setDataProvider( providerKey );

    // create and bind a renderer to this layer

    QDomNode singlenode = layer_node.namedItem("singlesymbol");
    QDomNode graduatednode = layer_node.namedItem("graduatedsymbol");
    QDomNode continuousnode = layer_node.namedItem("continuoussymbol");
    QDomNode singlemarkernode = layer_node.namedItem("singlemarker");
    QDomNode graduatedmarkernode = layer_node.namedItem("graduatedmarker");

    std::auto_ptr<QgsRenderer> renderer;

    if (!singlenode.isNull())
    {
        renderer.reset( new QgsSingleSymRenderer );
        renderer->readXML(singlenode, *this);
    }
    else if (!graduatednode.isNull())
    {
        renderer.reset( new QgsGraduatedSymRenderer );
        renderer->readXML(graduatednode, *this);
    }
    else if (!continuousnode.isNull())
    {
        renderer.reset( new QgsContinuousColRenderer );
        renderer->readXML(continuousnode, *this);
    }
    else if(!singlemarkernode.isNull())
    {
        renderer.reset( new QgsSiMaRenderer );
        renderer->readXML(singlemarkernode, *this);
    }
    else if(!graduatedmarkernode.isNull())
    {
        renderer.reset( new QgsGraduatedMaRenderer );
        renderer->readXML(graduatedmarkernode, *this);
    }

    return valid;               // should be true if read successfully

} // void QgsVectorLayer::readXML_



void
QgsVectorLayer:: setDataProvider( QString const & provider )
{
    // XXX should I check for and possibly delete any pre-existing providers?
    // XXX How often will that scenario occur?

    providerKey = provider;     // XXX is this necessary?  Usually already set
                                // XXX when execution gets here.

    // load the plugin
    QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
    QString ogrlib = pReg->library(provider);

    //QString ogrlib = libDir + "/libpostgresprovider.so";

    const char *cOgrLib = (const char *) ogrlib;

#ifdef TESTPROVIDERLIB
    // test code to help debug provider loading problems
//  void *handle = dlopen(cOgrLib, RTLD_LAZY);
    void *handle = dlopen(cOgrLib, RTLD_LAZY | RTLD_GLOBAL);
    if (!handle)
    {
        std::cout << "Error in dlopen: " << dlerror() << std::endl;

    } else
    {
        std::cout << "dlopen suceeded" << std::endl;
        dlclose(handle);
    }

#endif

    // load the data provider
    myLib = new QLibrary((const char *) ogrlib);
#ifdef QGISDEBUG
    std::cout << "Library name is " << myLib->library() << std::endl;
#endif
    bool loaded = myLib->load();

    if (loaded)
    {
#ifdef QGISDEBUG
        std::cout << "Loaded data provider library" << std::endl;
        std::cout << "Attempting to resolve the classFactory function" << std::endl;
#endif
        create_it * classFactory = (create_it *) myLib->resolve("classFactory");

        valid = false;            // assume the layer is invalid until we
                                  // determine otherwise
        if (classFactory)
        {
#ifdef QGISDEBUG
            std::cout << "Getting pointer to a dataProvider object from the library\n";
#endif
            dataProvider = classFactory(dataSource);

            if (dataProvider)
            {
#ifdef QGISDEBUG
                std::cout << "Instantiated the data provider plugin\n";
#endif
                if (dataProvider->isValid())
                {
                    valid = true;

                    // get the extent
                    QgsRect *mbr = dataProvider->extent();

                    // show the extent
                    QString s = mbr->stringRep();
#ifdef QGISDEBUG
                    std::cout << "Extent of layer: " << s << std::endl;
#endif
                    // store the extent
                    layerExtent.setXmax(mbr->xMax());
                    layerExtent.setXmin(mbr->xMin());
                    layerExtent.setYmax(mbr->yMax());
                    layerExtent.setYmin(mbr->yMin());

                    // get and store the feature type
                    geometryType = dataProvider->geometryType();

                    // look at the fields in the layer and set the primary
                    // display field using some real fuzzy logic
                    setDisplayField();

                    if (providerKey == "postgres")
                    {
#ifdef QGISDEBUG
                        std::cout << "Beautifying layer name " << layerName << std::endl;
#endif
                        // adjust the display name for postgres layers
                        layerName = layerName.mid(layerName.find(".") + 1);
                        layerName = layerName.left(layerName.find("("));
#ifdef QGISDEBUG
                        std::cout << "Beautified name is " << layerName << std::endl;
#endif
                    }

                    // upper case the first letter of the layer name
                    layerName = layerName.left(1).upper() + layerName.mid(1);
  
                    // label
                    mLabel = new QgsLabel ( dataProvider->fields() ); 
                    mLabelOn = false;
                }
            } else
            {
#ifdef QGISDEBUG
                std::cout << "Unable to instantiate the data provider plugin\n";
#endif
                valid = false;
            }
        }
    } else
    {
        valid = false;
#ifdef QGISDEBUG
        std::cout << "Failed to load " << "../providers/libproviders.so" << "\n";
#endif
    }

} // QgsVectorLayer:: setDataProvider
