/***************************************************************************
                          qgisapp.cpp  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
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
 /* $Id$ */
 
#include <dlfcn.h>

#include <qapplication.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qcanvas.h>
#include <qcolor.h>
#include <qdir.h>
#include <qscrollview.h>
#include <qstringlist.h>
#include <qmessagebox.h>
#include <qerrormessage.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qsplitter.h>
#include <qpopupmenu.h>
#include <qrect.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qlayout.h>
#include <qwmatrix.h>
#include <qfiledialog.h>
#include <qlibrary.h>
#include <qvbox.h>
#include <qlistview.h>
#include <qsettings.h>
#include <qtextstream.h>
#include <qsocket.h>

#include <iostream>
#include <iomanip>
#include "qgsrect.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgslegenditem.h"
#include "qgslegend.h"
#include "qgsprojectio.h"
#ifdef POSTGRESQL
#include <libpq++.h>
#include "qgsdbsourceselect.h"
#include "qgsdatabaselayer.h"
#endif
#include "qgsmessageviewer.h"
#include "qgsshapefilelayer.h"
#include "qgsvectorlayer.h"
#include "qgslayerproperties.h"
#include "qgsabout.h"
#include "qgspluginmanager.h"
#include "qgsmaplayerinterface.h"
#include "qgis.h"
#include "qgisapp.h"
#include "qgspluginitem.h"
#include "../plugins/qgisplugin.h"
#include "xpm/qgis.xpm"
#include <ogrsf_frmts.h>
typedef QgsMapLayerInterface* create_it();
typedef QString name_t();
typedef QString description_t();

// version
static const char *qgisVersion = "0.0.13 Data Provider Branch - September 2003";
static const int qgisVersionInt = 13;
// cursors
static unsigned char zoom_in_bits[] = {
	0xf8, 0x00, 0x06, 0x03, 0x22, 0x02, 0x21, 0x04, 0x21, 0x04, 0xfd, 0x05,
	0x21, 0x04, 0x21, 0x04, 0x22, 0x02, 0x06, 0x07, 0xf8, 0x0e, 0x00, 0x3c,
	0x00, 0x78, 0x00, 0xf8, 0x00, 0xf0, 0x00, 0xe0
};

static unsigned char zoom_in_mask_bits[] = {
	0xf8, 0x00, 0x06, 0x03, 0x22, 0x02, 0x21, 0x04, 0x21, 0x04, 0xfd, 0x05,
	0x21, 0x04, 0x21, 0x04, 0x22, 0x02, 0x06, 0x07, 0xf8, 0x0e, 0x00, 0x3c,
	0x00, 0x78, 0x00, 0xf8, 0x00, 0xf0, 0x00, 0xe0
};

static unsigned char zoom_out_bits[] = {
	0xf8, 0x00, 0x06, 0x03, 0x02, 0x02, 0x01, 0x04, 0x01, 0x04, 0xfd, 0x05,
	0x01, 0x04, 0x01, 0x04, 0x02, 0x02, 0x06, 0x07, 0xf8, 0x0e, 0x00, 0x3c,
	0x00, 0x78, 0x00, 0xf8, 0x00, 0xf0, 0x00, 0xe0
};

static unsigned char pan_bits[] = {
	0xf0, 0x00, 0xf8, 0x01, 0x28, 0x07, 0x28, 0x05, 0x28, 0x1d, 0x28, 0x15,
	0x2f, 0x15, 0x0d, 0x14, 0x09, 0x10, 0x03, 0x18, 0x06, 0x08, 0x04, 0x08,
	0x0c, 0x0c, 0x18, 0x04, 0x30, 0x04, 0xe0, 0x07
};

static unsigned char pan_mask_bits[] = {
	0xf0, 0x00, 0xf8, 0x01, 0xf8, 0x07, 0xf8, 0x07, 0xf8, 0x1f, 0xf8, 0x1f,
	0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xfe, 0x0f, 0xfc, 0x0f,
	0xfc, 0x0f, 0xf8, 0x07, 0xf0, 0x07, 0xe0, 0x07
};

static unsigned char identify_bits[] = {
	0x00, 0x00, 0x06, 0x00, 0x1e, 0x00, 0x3c, 0x00, 0xfc, 0x00, 0xf8, 0x01,
	0x78, 0x00, 0xd8, 0x1c, 0x88, 0x37, 0x80, 0x7f, 0x80, 0xf7, 0x80, 0xf7,
	0x80, 0xf7, 0x00, 0x63, 0x00, 0x3e, 0x00, 0x1c
};

static unsigned char identify_mask_bits[] = {
	0x00, 0x00, 0x06, 0x00, 0x1e, 0x00, 0x3c, 0x00, 0xfc, 0x00, 0xf8, 0x01,
	0x78, 0x00, 0xd8, 0x1c, 0x88, 0x3f, 0x80, 0x7f, 0x80, 0xff, 0x80, 0xff,
	0x80, 0xff, 0x00, 0x7f, 0x00, 0x3e, 0x00, 0x1c
};

static unsigned char select_cursor_bits[] = {
  0x00, 0x00, 0xfe, 0x00, 0x7e, 0x00, 0x1e, 0x00, 0x3e, 0x00, 0x76, 0x00,
  0xf6, 0x55, 0xc2, 0x01, 0x90, 0x43, 0x00, 0x07, 0x10, 0x4e, 0x00, 0x0c,
  0x50, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


static unsigned char select_cursor_mask_bits[] = {
      0x00, 0x00, 0xfe, 0x00, 0x7e, 0x00, 0x1e, 0x00, 0x3e, 0x00, 0x76, 0x00,
   0xf6, 0x7f, 0xf2, 0x7f, 0xf0, 0x7f, 0xf0, 0x7f, 0xf0, 0x7f, 0xf0, 0x7f,
   0xf0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

};

// constructor starts here   

QgisApp::QgisApp(QWidget * parent, const char *name, WFlags fl):QgisAppBase(parent, name, fl)
{
	OGRRegisterAll();
	QPixmap icon;
	icon = QPixmap(qgis_xpm);
	setIcon(icon);
	// store startup location
	QDir *d = new QDir();
	startupPath = d->absPath();
	delete d;
	QBitmap zoomincur;
//  zoomincur = QBitmap(cursorzoomin);
	QBitmap zoomincurmask;
//  zoomincurmask = QBitmap(cursorzoomin_mask);

	QGridLayout *FrameLayout = new QGridLayout(frameMain, 1, 2, 4, 6, "mainFrameLayout");
	QSplitter *split = new QSplitter(frameMain);
	legendView = new QListView(split);
	legendView->addColumn("Layers");
	legendView->setSorting(-1);


	mapLegend = new QgsLegend(legendView);	//frameMain);
	// mL = new QScrollView(split);
	//add a canvas
	mapCanvas = new QgsMapCanvas(split);
	// resize it to fit in the frame
	//    QRect r = frmCanvas->rect();
	//    canvas->resize(r.width(), r.height());
	mapCanvas->setBackgroundColor(Qt::white);	//QColor (220, 235, 255));
	mapCanvas->setMinimumWidth(400);
	FrameLayout->addWidget(split, 0, 0);
	mapLegend->setBackgroundColor(QColor(192, 192, 192));
	mapLegend->setMapCanvas(mapCanvas);
	legendView->setResizeMode(QListView::AllColumns);
	QString caption = "Quantum GIS - ";
	caption += qgisVersion;
	setCaption(caption);
	connect(mapCanvas, SIGNAL(xyCoordinates(QgsPoint &)), this, SLOT(showMouseCoordinate(QgsPoint &)));
	connect(legendView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(layerProperties(QListViewItem *)));
	connect(legendView, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
			this, SLOT(rightClickLegendMenu(QListViewItem *, const QPoint &, int)));

	// create the layer popup menu
	popMenu = new QPopupMenu();
	popMenu->insertItem("&Zoom to extent of selected layer", this, SLOT(zoomToLayerExtent()));
	popMenu->insertItem("&Open attribute table", this, SLOT(attributeTable()));
	popMenu->insertSeparator();
	popMenu->insertItem("&Properties", this, SLOT(layerProperties()));
	popMenu->insertSeparator();
	popMenu->insertItem("&Remove", this, SLOT(removeLayer()));
	mapCursor = 0;
	// create the interfce
	qgisInterface = new QgisIface(this);
	///qgisInterface->setParent(this);
	// set the legend control for the map canvas
	mapCanvas->setLegend(mapLegend);
	// disable functions based on build type
	#ifndef POSTGRESQL
		actionAddLayer->removeFrom(PopupMenu_2);
		actionAddLayer->removeFrom(DataToolbar);
	#endif
	// connect the "cleanup" slot
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveWindowState()));
	restoreWindowState();
}

QgisApp::~QgisApp()
{
}
void QgisApp::about()
{
	QgsAbout *abt = new QgsAbout();
	QString versionString = "Version ";
	versionString += qgisVersion;
	#ifdef POSTGRESQL
		versionString += " with PostgreSQL support";
	#else
		versionString +=  " (no PostgreSQL support)";
	#endif
	abt->setVersion(versionString);
	QString urls = "Web Page: http://qgis.sourceforge.net\nSourceforge Project "
		"Page: http://sourceforge.net/projects/qgis";
	abt->setURLs(urls);
	QString watsNew = "Version ";
	watsNew += qgisVersion;
	watsNew += "\n**Multiple features displayed with the Identify tool\n" 
	"**Preliminary Plugin Manager implementation\n"
	 "**Version check under tools menu\n"
	 "**Version checking uses port 80 to prevent problems with firewalls\n"
	 "**Fix for PostGIS bug when srid != -1\n"
	 "**Fix for PostGIS LINESTRING rendering\n"
	 "**Database Connections can now be deleted\n"
	 "**Fixes to Database Connection dialog\n"
	 "**Fix for crash when opening a shapefile attribute table twice in succession\n"
	 "**Fix for crash when opening invalid shapefiles\n"
	 
		;


	abt->setWhatsNew(watsNew);
	abt->exec();

}

void QgisApp::addLayer()
{

	mapCanvas->freeze();
	QStringList files = QFileDialog::getOpenFileNames("Shapefiles (*.shp);;All files (*.*)", 0, this, "open files dialog",
													  "Select one or more layers to add");
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QStringList::Iterator it = files.begin();
	while (it != files.end()) {


		QFileInfo fi(*it);
		QString base = fi.baseName();


		// create the layer

	//dp	QgsShapeFileLayer *lyr = new QgsShapeFileLayer(*it, base);
    	QgsVectorLayer *lyr = new QgsVectorLayer(*it, base);
		QObject::connect(lyr,SIGNAL(repaintRequested()),mapCanvas,SLOT(refresh()));
		// give it a random color

		if (lyr->isValid()) {
			// add it to the mapcanvas collection
			mapCanvas->addLayer(lyr);
		} else {
			QString msg = *it;
			msg += " is not a valid or recognized data source";
			QMessageBox::critical(this, "Invalid Data Source", msg);
		}

		++it;
	}
	//qApp->processEvents();
	// update legend
	/*! \todo Need legend scrollview and legenditem classes */
	// draw the map

	mapLegend->update();
	qApp->processEvents();
	mapCanvas->freeze(false);
	mapCanvas->render2();
	QApplication::restoreOverrideCursor();
	statusBar()->message(mapCanvas->extent().stringRep());



}
#ifdef POSTGRESQL
void QgisApp::addDatabaseLayer()
{
	// only supports postgis layers at present
	// show the postgis dialog


	QgsDbSourceSelect *dbs = new QgsDbSourceSelect(this);
	mapCanvas->freeze();
	if (dbs->exec()) {
		QApplication::setOverrideCursor(Qt::WaitCursor);


		// repaint the canvas if it was covered by the dialog

		// add files to the map canvas
		QStringList tables = dbs->selectedTables();
		
		QString connInfo = dbs->connInfo();
		// for each selected table, connect to the database, parse the WKT geometry,
		// and build a cavnasitem for it
		// readWKB(connInfo,tables);
		QStringList::Iterator it = tables.begin();
		while (it != tables.end()) {

			// create the layer
			//qWarning("creating lyr");
			QgsDatabaseLayer *lyr = new QgsDatabaseLayer(connInfo, *it);
			// give it a random color

			// add it to the mapcanvas collection
			mapCanvas->addLayer(lyr);
			//qWarning("incrementing iterator");
			++it;
		}
		//  qApp->processEvents();
		// update legend
		/*! \todo Need legend scrollview and legenditem classes */
		mapLegend->update();

		// draw the map
		//mapCanvas->render2();
		statusBar()->message(mapCanvas->extent().stringRep());

	}
	qApp->processEvents();

	mapCanvas->freeze(false);
	mapCanvas->render2();
	QApplication::restoreOverrideCursor();

}
#endif
void QgisApp::fileExit()
{
	QApplication::exit();

}
void QgisApp::fileNew(){
	mapCanvas->removeAll();
	setCaption("Quantum GIS -- Untitled");
	mapCanvas->clear();
	mapLegend->update();
	fullPath = "";
}
void QgisApp::fileOpen(){
	mapCanvas->freeze(true);
	QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::OPEN);
	
	if(pio->read()){
		setCaption("Quantum GIS -- " + pio->baseName());
		fullPath = pio->fullPathName();
		}
	delete pio;
	
	mapLegend->update();
	mapCanvas->freeze(false);
}
void QgisApp::fileSave(){
	QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::SAVE);
	pio->setFileName(fullPath);
	if(pio->write()){
		setCaption("Quantum GIS -- " + pio->baseName());
		statusBar()->message("Saved map to: " + pio->fullPathName());
	}
	delete pio;
}

void QgisApp::fileSaveAs(){
	QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::SAVEAS);
	if(pio->write()){
		setCaption("Quantum GIS -- " + pio->baseName());
		statusBar()->message("Saved map to: " + pio->fullPathName());
	}
	delete pio;	
}

void QgisApp::zoomIn()
{
	/*  QWMatrix m = mapCanvas->worldMatrix();
	   m.scale( 2.0, 2.0 );
	   mapCanvas->setWorldMatrix( m );
	 */

	mapTool = QGis::ZoomIn;
	mapCanvas->setMapTool(mapTool);
	// set the cursor
	QBitmap zoomInBmp(16, 16, zoom_in_bits, true);
	QBitmap zoomInBmpMask(16, 16, zoom_in_mask_bits, true);
	delete mapCursor;
	mapCursor = new QCursor(zoomInBmp, zoomInBmpMask, 5, 5);
	mapCanvas->setCursor(*mapCursor);
	// scale the extent
	/* QgsRect ext = mapCanvas->extent();
	   ext.scale(0.5);
	   mapCanvas->setExtent(ext);
	   statusBar()->message(ext.stringRep());
	   mapCanvas->clear();
	   mapCanvas->render2(); */

}

void QgisApp::zoomOut()
{
	mapTool = QGis::ZoomOut;
	mapCanvas->setMapTool(mapTool);

	QBitmap zoomOutBmp(16, 16, zoom_out_bits, true);
	delete mapCursor;
	mapCursor = new QCursor(zoomOutBmp, zoomOutBmp, 5, 5);
	mapCanvas->setCursor(*mapCursor);

	/*    QWMatrix m = mapCanvas->worldMatrix();
	   m.scale( 0.5, 0.5 );
	   mapCanvas->setWorldMatrix( m );
	 */


}

void QgisApp::pan()
{
	mapTool = QGis::Pan;
	mapCanvas->setMapTool(mapTool);
	QBitmap panBmp(16, 16, pan_bits, true);
	QBitmap panBmpMask(16, 16, pan_mask_bits, true);
	delete mapCursor;
	mapCursor = new QCursor(panBmp, panBmpMask, 5, 5);
	mapCanvas->setCursor(*mapCursor);
}

void QgisApp::zoomFull()
{
	mapCanvas->zoomFullExtent();
}

void QgisApp::zoomPrevious()
{
	mapCanvas->zoomPreviousExtent();
}

void QgisApp::identify()
{
	mapTool = QGis::Identify;
	mapCanvas->setMapTool(mapTool);
	QBitmap idenitfyBmp(16, 16, identify_bits, true);
	QBitmap identifyBmpMask(16, 16, identify_mask_bits, true);
	delete mapCursor;
	mapCursor = new QCursor(idenitfyBmp, identifyBmpMask, 1, 1);
	mapCanvas->setCursor(*mapCursor);
}

void QgisApp::attributeTable()
{
	QListViewItem *li = legendView->currentItem();
	if(li){
	QgsMapLayer *lyr = ((QgsLegendItem *) li)->layer();
		if (lyr) {
			lyr->table();
	
		} else {
			QMessageBox::information(this, "No Layer Selected", "To open an attribute table, you must select a layer in the legend");
		}
	}
}

void QgisApp::select()
{
	// set the select cursor
	QBitmap selectBmp(16, 16, select_cursor_bits, true);
	QBitmap selectBmpMask(16, 16, select_cursor_mask_bits, true);
	delete mapCursor;
	mapCursor = new QCursor(selectBmpMask, selectBmp, 1, 1);
	mapCanvas->setCursor(*mapCursor);
	// set current map tool to select
    mapCanvas->setMapTool(QGis::Select);
}

//void QgisApp::readWKB (const char *connInfo, QStringList tables)
//{
//    PgCursor pgc (connInfo, "testcursor");
//   // get "endianness"
//   char *chkEndian = new char[4];
//   memset (chkEndian, '\0', 4);
//   chkEndian[0] = 0xE8;
//   int *ce = (int *) chkEndian;
//   bool isNDR = (232 == *ce);
//   /*     if(*ce != 232)
//   cout << "Big endian" << endl;
//   else
//   cout << "Little endian" << endl;
//   */
//   QStringList::Iterator it = tables.begin ();
//   while (it != tables.end ())
//     {

//       // get the extent of the layer
//       QString esql = "select extent(the_geom) from " + *it;
//       PgDatabase *pd = new PgDatabase (connInfo);
//       int result = pd->ExecTuplesOk ((const char *) esql);
//       QString extent = pd->GetValue (0, 0);
//       // parse out the x and y values
//       extent = extent.right (extent.length () - extent.find ("BOX3D(") - 6);
//       QStringList coordPairs = QStringList::split (",", extent);
//       QStringList x1y1 = QStringList::split (" ", coordPairs[0]);
//       QStringList x2y2 = QStringList::split (" ", coordPairs[1]);
//       double x1 = x1y1[0].toDouble ();
//       double y1 = x1y1[1].toDouble ();
//       double x2 = x2y2[0].toDouble ();
//       double y2 = x2y2[1].toDouble ();
//       double xMu = x2 - x1;
//       double yMu = y2 - y1;
//       int subordinantAxisLength;


//       // determine the dominate direction for the mapcanvas
//       if (mapCanvas->width () > mapCanvas->height ())
//      {
//        subordinantAxisLength = mapCanvas->height ();
//        scaleFactor = yMu / subordinantAxisLength;
//        mapWindow = new QRect (x1, y1, xMu, xMu);
//      }
//       else
//      {
//        subordinantAxisLength = mapCanvas->width ();
//        scaleFactor = xMu / subordinantAxisLength;
//        mapWindow = new QRect (x1, y1, yMu, yMu);
//      }

//       const char *xtent = (const char *) extent;
//       string sql = "select asbinary(the_geom,";
//       if (isNDR)
//      sql += "'NDR'";
//       else
//      sql += "'XDR'";
//       sql += ") as features from ";
//       sql += *it++;
//       cout << sql.c_str () << endl;
//       pgc.Declare (sql.c_str (), true);
//       int res = pgc.Fetch ();
//       cout << "Number of binary records: " << pgc.Tuples () << endl;
//       bool setExtent = true;
//       // process each record
//       QPainter paint;

//       paint.begin (mapCanvas);
//       paint.setWindow (*mapWindow);
//       QRect v = paint.viewport ();
//       int d = QMIN (v.width (), v.height ());
//       paint.setViewport (v.left () + (v.width () - d) / 2,
//                       v.top () + (v.height () - d) / 2, d, d);


//       paint.setPen (Qt::red);

//       for (int idx = 0; idx < pgc.Tuples (); idx++)
//      {
//        cout << "Size of this record: " << pgc.GetLength (idx, 0) << endl;
//        // allocate memory for the item
//        char *feature = new char[pgc.GetLength (idx, 0) + 1];
//        memset (feature, '\0', pgc.GetLength (idx, 0) + 1);
//        memcpy (feature, pgc.GetValue (idx, 0), pgc.GetLength (idx, 0));


//        cout << "Endian is: " << (int) feature[0] << endl;
//        cout << "Geometry type is: " << (int) feature[1] << endl;
//        // print the x,y coordinates
//        double *x = (double *) (feature + 5);
//        double *y = (double *) (feature + 5 + sizeof (double));
//        cout << "x,y: " << setprecision (16) << *x << ", " << *y << endl;
//        QPoint pt = paint.xForm (QPoint ((int) *x, (int) *y));
//        cout << "Plotting " << *x << ", " << *y << " at " << pt.
//          x () << ", " << pt.y () << endl;
//        paint.drawRect ((int) *x, mapWindow->bottom () - (int) *y, 15000,
//                        15000);
//        // free it 
//        delete[]feature;
//      }
//       paint.end ();
//     }

//}


void QgisApp::drawPoint(double x, double y)
{
	QPainter paint;
	//  QWMatrix mat (scaleFactor, 0, 0, scaleFactor, 0, 0);
	paint.begin(mapCanvas);
	// paint.setWorldMatrix(mat);
	paint.setWindow(*mapWindow);

	paint.setPen(Qt::blue);
	paint.drawPoint((int) x, (int) y);
	paint.end();
}

void QgisApp::drawLayers()
{
//  std::cout << "In  QgisApp::drawLayers()" << std::endl;
	mapCanvas->setDirty(true);
	mapCanvas->render2();
}

void QgisApp::showMouseCoordinate(QgsPoint & p)
{
	statusBar()->message(p.stringRep());
	//qWarning("X,Y is: " + p.stringRep());

}

void QgisApp::testButton()
{
	QgsShapeFileLayer *sfl = new QgsShapeFileLayer("foo");
	mapCanvas->addLayer(sfl);
//      delete sfl;

}

void QgisApp::layerProperties()
{
	layerProperties(legendView->currentItem());
}

void QgisApp::layerProperties(QListViewItem * lvi)
{
	QgsMapLayer *lyr;
	if (lvi)
		lyr = ((QgsLegendItem *) lvi)->layer();
	else {
		// get the selected item
		QListViewItem *li = legendView->currentItem();

		lyr = ((QgsLegendItem *) li)->layer();
	}
	QString currentName = lyr->name();
	QgsLayerProperties *lp = new QgsLayerProperties(lyr);
	if (lp->exec()) {
		// update the symbol
		lyr->setSymbol(lp->getSymbol());
		mapCanvas->freeze();
		lyr->setlayerName(lp->displayName());
		if (currentName != lp->displayName())
			mapLegend->update();
		delete lp;
		qApp->processEvents();

		// apply changes
		mapCanvas->freeze(false);
		mapCanvas->setDirty(true);
		mapCanvas->render2();
	}

}
void QgisApp::removeLayer()
{
	mapCanvas->freeze();
	QListViewItem *lvi = legendView->currentItem();
	QgsMapLayer *lyr = ((QgsLegendItem *) lvi)->layer();
	mapCanvas->remove(lyr->getLayerID());
	mapLegend->update();
	mapCanvas->freeze(false);
	// draw the map
	mapCanvas->clear();
	mapCanvas->render2();


}

void QgisApp::zoomToLayerExtent()
{

	// get the selected item
	QListViewItem *li = legendView->currentItem();
	QgsMapLayer *lyr = ((QgsLegendItem *) li)->layer();
	mapCanvas->setExtent(lyr->extent());
	mapCanvas->clear();
	mapCanvas->render2();

}

void QgisApp::rightClickLegendMenu(QListViewItem * lvi, const QPoint & pt, int)
{
	if (lvi)
		popMenu->exec(pt);
}

QgisIface * QgisApp::getInterface(){
	return qgisInterface;
}
int QgisApp::getInt(){
	return 99;
}
void QgisApp::actionPluginManager_activated(){
	QgsPluginManager *pm = new QgsPluginManager(this);
	if(pm->exec()){
		// load selected plugins
		std::vector<QgsPluginItem> pi = pm->getSelectedPlugins();
		std::vector<QgsPluginItem>::iterator it = pi.begin();
		while(it != pi.end()){
			QgsPluginItem plugin = *it;
			loadPlugin(plugin.name(), plugin.description(), plugin.fullPath());
			it++;
		}

		
	}
	

}
void QgisApp::loadPlugin(QString name, QString description, QString fullPath){
	QLibrary *myLib = new QLibrary(fullPath);
		std::cout << "Library name is " << myLib->library() << std::endl;
		bool loaded = myLib->load();
		if (loaded) {
			std::cout << "Loaded test plugin library" << std::endl;
			std::cout << "Attempting to resolve the classFactory function" << std::endl;
			create_it *cf = (create_it *) myLib->resolve("classFactory");
	
			if (cf) {
				std::cout << "Getting pointer to a MapLayerInterface object from the library\n";
				QgsMapLayerInterface *pl = cf();
				if(pl){
					std::cout << "Instantiated the maplayer test plugin\n";
					// set the main window pointer for the plugin
					pl->setQgisMainWindow(this);
					std::cout << "getInt returned " << pl->getInt() << " from map layer plugin\n";
					// set up the gui
					pl->initGui();
				}else{
					std::cout << "Unable to instantiate the maplayer test plugin\n";
				}
			}
		}else{
			std::cout << "Failed to load " << fullPath << "\n";
		}
}
void QgisApp::testMapLayerPlugins(){
	// map layer plugins live in their own directory (somewhere to be determined)
	QDir mlpDir("../plugins/maplayer", "*.so.1.0.0", QDir::Name | QDir::IgnoreCase, QDir::Files );
	if(mlpDir.count() == 0){
		QMessageBox::information(this,"No MapLayer Plugins", "No MapLayer plugins in ../plugins/maplayer");
	}else{
		for(unsigned i = 0; i < mlpDir.count(); i++){
		std::cout << "Getting information for plugin: " << mlpDir[i] << std::endl;
		std::cout << "Attempting to load the plugin using dlopen\n";
		void *handle = dlopen("../plugins/maplayer/" + mlpDir[i], RTLD_LAZY);
	             if (!handle) {
			std::cout << "Error in dlopen: " <<  dlerror() << std::endl;
		     }else{
			std::cout << "dlopen suceeded" << std::endl;
			dlclose(handle);
		     }
		
		QLibrary *myLib = new QLibrary("../plugins/maplayer/" + mlpDir[i]);
		std::cout << "Library name is " << myLib->library() << std::endl;
		bool loaded = myLib->load();
		if (loaded) {
			std::cout << "Loaded test plugin library" << std::endl;
			std::cout << "Attempting to resolve the classFactory function" << std::endl;
			create_it *cf = (create_it *) myLib->resolve("classFactory");
	
			if (cf) {
				std::cout << "Getting pointer to a MapLayerInterface object from the library\n";
				QgsMapLayerInterface *pl = cf();
				if(pl){
					std::cout << "Instantiated the maplayer test plugin\n";
					// set the main window pointer for the plugin
					pl->setQgisMainWindow(this);
					std::cout << "getInt returned " << pl->getInt() << " from map layer plugin\n";
					// set up the gui
					pl->initGui();
				}else{
					std::cout << "Unable to instantiate the maplayer test plugin\n";
				}
			}
		}else{
			std::cout << "Failed to load " << mlpDir[i] << "\n";
		}
	}
}
}
void QgisApp::testPluginFunctions()
{
	// test maplayer plugins first
	testMapLayerPlugins();
	if(false){
// try to load plugins from the plugin directory and test each one

	QDir pluginDir("../plugins", "*.so*", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);
	//pluginDir.setFilter(QDir::Files || QDir::NoSymLinks);
	//pluginDir.setNameFilter("*.so*");
	if(pluginDir.count() == 0){
		QMessageBox::information(this, "No Plugins", "No plugins found in ../plugins. To test plugins, start qgis from the src directory");
	}else{
		
		for(unsigned i = 0; i < pluginDir.count(); i++){
		std::cout << "Getting information for plugin: " << pluginDir[i] << std::endl;
		QLibrary *myLib = new QLibrary("../plugins/" + pluginDir[i]);//"/home/gsherman/development/qgis/plugins/" + pluginDir[i]);
		std::cout << "Library name is " << myLib->library() << std::endl;
		//QLibrary myLib("../plugins/" + pluginDir[i]);
		std::cout << "Attempting to load " << + "../plugins/" + pluginDir[i] << std::endl;
	/*	void *handle = dlopen("/home/gsherman/development/qgis/plugins/" + pluginDir[i], RTLD_LAZY);
	             if (!handle) {
                      std::cout << "Error in dlopen: " <<  dlerror() << std::endl;
                     
                  }else{
				  	std::cout << "dlopen suceeded" << std::endl;
					dlclose(handle);
					}
				  	
*/
		bool loaded = myLib->load();
		if (loaded) {
			std::cout << "Loaded test plugin library" << std::endl;
			std::cout << "Getting the name of the plugin" << std::endl;
			name_t *pName = (name_t *) myLib->resolve("name");
			if(pName){
				QMessageBox::information(this,"Name", "Plugin " + pluginDir[i] + " is named " + pName());
			}
			std::cout << "Attempting to resolve the classFactory function" << std::endl;
			create_t *cf = (create_t *) myLib->resolve("classFactory");
	
			if (cf) {
				std::cout << "Getting pointer to a QgisPlugin object from the library\n";
				QgisPlugin *pl = cf(this, qgisInterface);
				std::cout << "Displaying name, version, and description\n";
				std::cout << "Plugin name: " << pl->name() << std::endl;
				std::cout << "Plugin version: " << pl->version() << std::endl;
				std::cout << "Plugin description: " << pl->description() << std::endl;
				QMessageBox::information(this, "Plugin Information", "QGis loaded the following plugin:\nName: "
										 + pl->name() + "\nVersion: " + pl->version() + "\nDescription: " + pl->description());
				// unload the plugin (delete it)
				std::cout << "Attempting to resolve the unload function" << std::endl;
				/*
				unload_t *ul = (unload_t *) myLib.resolve("unload");
				if (ul) {
					ul(pl);
					std::cout << "Unloaded the plugin\n";
				} else {
					std::cout << "Unable to resolve unload function. Plugin was not unloaded\n";
				}
				*/
			}
		} else {
			QMessageBox::warning(this, "Unable to Load Plugin",
								 "QGis was unable to load the plugin from: ../plugins/" + pluginDir[i]);
			std::cout << "Unable to load library" << std::endl;
		}
		}
	}
	}
}

void QgisApp::saveWindowState()
{
	// store window and toolbar positions
	QSettings settings;

	QString dockStatus;
	QTextStream ts(&dockStatus, IO_WriteOnly);
	ts << *this;
	settings.writeEntry("/qgis/Geometry/ToolBars", dockStatus);
	// store window geometry
	QPoint p = this->pos();
	QSize s = this->size();

	settings.writeEntry("/qgis/Geometry/maximized", this->isMaximized());
	settings.writeEntry("/qgis/Geometry/x", p.x());
	settings.writeEntry("/qgis/Geometry/y", p.y());
	settings.writeEntry("/qgis/Geometry/w", s.width());
	settings.writeEntry("/qgis/Geometry/h", s.height());

}

void QgisApp::restoreWindowState()
{
	QSettings settings;

	QString dockStatus = settings.readEntry("/qgis/Geometry/ToolBars");
	QTextStream ts(&dockStatus, IO_ReadOnly);
	ts >> *this;



	// restore window geometry
	QDesktopWidget *d = QApplication::desktop();
	int dw = d->width();		// returns desktop width
	int dh = d->height();		// returns desktop height
	int w = settings.readNumEntry("/qgis/Geometry/w", 600);
	int h = settings.readNumEntry("/qgis/Geometry/h", 400);
	int x = settings.readNumEntry("/qgis/Geometry/x", (dw - 600) / 2);
	int y = settings.readNumEntry("/qgis/Geometry/y", (dh - 400) / 2);
	setGeometry(x, y, w, h);
}
void QgisApp::checkQgisVersion(){
QApplication::setOverrideCursor(Qt::WaitCursor);
/* QUrlOperator op = new QUrlOperator( "http://mrcc.com/qgis/version.txt" );
connect(op, SIGNAL(data()), SLOT(urlData()));
connect(op, SIGNAL(finished(QNetworkOperation)), SLOT(urlFinished(QNetworkOperation)));

op.get(); */
		socket = new QSocket( this );
        connect( socket, SIGNAL(connected()),
                SLOT(socketConnected()) );
        connect( socket, SIGNAL(connectionClosed()),
                SLOT(socketConnectionClosed()) );
		connect( socket, SIGNAL(readyRead()),
                SLOT(socketReadyRead()) );
		connect( socket, SIGNAL(error(int)),
                SLOT(socketError(int)) );
		socket->connectToHost("mrcc.com", 80);
}
void QgisApp::socketConnected(){
		QTextStream os(socket);
		versionMessage = "";
		// send the qgis version string
       // os << qgisVersion << "\r\n";
	   os << "GET /qgis/version.txt HTTP/1.0\n\n";
		
	
}
void QgisApp::socketConnectionClosed(){
	QApplication::restoreOverrideCursor();
	// strip the header
	QString contentFlag = "#QGIS Version";
	int pos = versionMessage.find(contentFlag);
	if(pos >-1){
		pos += contentFlag.length();
		/* std::cout << versionMessage << "\n ";
		std::cout << "Pos is " << pos <<"\n"; */
		versionMessage = versionMessage.mid(pos);
		QStringList parts = QStringList::split("|",versionMessage);
		// check the version from the  server against our version
		QString versionInfo;
		int currentVersion = parts[0].toInt();
		if(currentVersion > qgisVersionInt){
		// show version message from server
			versionInfo = "There is a new version of QGIS available\n";
		}else{
			if(qgisVersionInt > currentVersion){
				versionInfo = "You are running a development version of QGIS\n";
			}else{
			versionInfo = "You are running the current version of QGIS\n";
			}
		}
		if(parts.count() > 1){
			versionInfo += parts[1] + "\n\nWould you like more information?";;
			int result = QMessageBox::information(this,"QGIS Version Information", versionInfo, "Yes", "No");
			if(result ==0){
				// show more info
				QgsMessageViewer *mv = new QgsMessageViewer(this);
				mv->setCaption("QGIS - Changes in CVS");
				mv->setMessage(parts[2]);
				mv->exec();
			}	
		}else{
			QMessageBox::information(this, "QGIS Version Information", versionInfo);
		}
	}else{
		QMessageBox::warning(this, "QGIS Version Information", "Unable to get current version information from server");
	}
}
void QgisApp::socketError(int e){
		QApplication::restoreOverrideCursor();
// get errror type
QString detail;
switch(e){
	case QSocket::ErrConnectionRefused:
		detail = "Connection refused - server may be down";
		break;
	case QSocket::ErrHostNotFound:
		detail = "QGIS server was not found";
		break;
	case QSocket::ErrSocketRead:
		detail = "Error reading from server";
		break;
		}
	// show version message from server
	QMessageBox::critical(this, "QGIS Version Information", "Unable to connect to the QGIS Version server\n" + detail);
}

 void QgisApp::socketReadyRead()
    {
	while(socket->bytesAvailable() > 0){
		char *data = new char[socket->bytesAvailable() +1];
		memset(data, '\0', socket->bytesAvailable() +1);
		socket->readBlock(data, socket->bytesAvailable());
		versionMessage += data;
		delete[] data;
	}
           
    }
