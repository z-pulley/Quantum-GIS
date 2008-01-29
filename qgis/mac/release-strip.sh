#!/bin/sh
# Strip all non-global symbols

PREFIX=qgis0.9.2.app/Contents/MacOS

strip -x $PREFIX/qgis
strip -x $PREFIX/bin/qgis_help.app/Contents/MacOS/qgis_help

strip -x $PREFIX/lib/libqgis_core.dylib
strip -x $PREFIX/lib/libqgis_gui.dylib
strip -x $PREFIX/lib/libqgisgrass.dylib
strip -x $PREFIX/lib/qgis/libcopyrightlabelplugin.so 
strip -x $PREFIX/lib/qgis/libdelimitedtextplugin.so
strip -x $PREFIX/lib/qgis/libdelimitedtextprovider.so
strip -x $PREFIX/lib/qgis/libgeorefplugin.so 
strip -x $PREFIX/lib/qgis/libgpsimporterplugin.so
strip -x $PREFIX/lib/qgis/libgpxprovider.so
strip -x $PREFIX/lib/qgis/libgrassplugin.so 
strip -x $PREFIX/lib/qgis/libgrassprovider.so 
strip -x $PREFIX/lib/qgis/libgridmakerplugin.so 
strip -x $PREFIX/lib/qgis/libwfsprovider.so
strip -x $PREFIX/lib/qgis/libnortharrowplugin.so
strip -x $PREFIX/lib/qgis/libogrprovider.so
strip -x $PREFIX/lib/qgis/libpggeoprocessingplugin.so
strip -x $PREFIX/lib/qgis/libpostgresprovider.so
strip -x $PREFIX/lib/qgis/libquickprintplugin.so
strip -x $PREFIX/lib/qgis/libscalebarplugin.so
strip -x $PREFIX/lib/qgis/libspitplugin.so
strip -x $PREFIX/lib/qgis/libwfsplugin.so
strip -x $PREFIX/lib/qgis/libwmsprovider.so
#strip -x $PREFIX/lib/qgis/libopenmodellerplugin.so 

strip -x $PREFIX/lib/Qt3Support.framework/Versions/4/Qt3Support
strip -x $PREFIX/lib/QtCore.framework/Versions/4/QtCore
strip -x $PREFIX/lib/QtGui.framework/Versions/4/QtGui
strip -x $PREFIX/lib/QtNetwork.framework/Versions/4/QtNetwork
strip -x $PREFIX/lib/QtSql.framework/Versions/4/QtSql
strip -x $PREFIX/lib/QtSvg.framework/Versions/4/QtSvg
strip -x $PREFIX/lib/QtXml.framework/Versions/4/QtXml
strip -x $PREFIX/../plugins/imageformats/libqjpeg.dylib

strip -x $PREFIX/lib/libgdal.1.dylib
strip -x $PREFIX/lib/gdalplugins/gdal_GRASS.so
strip -x $PREFIX/lib/gdalplugins/ogr_GRASS.so
strip -x $PREFIX/lib/libgeos.3.0.0.dylib
strip -x $PREFIX/lib/libgeos_c.1.4.1.dylib
strip -x $PREFIX/lib/libproj.0.5.4.dylib
strip -x $PREFIX/lib/libsqlite3.0.8.6.dylib
strip -x $PREFIX/lib/libxerces-c.28.0.dylib
strip -x $PREFIX/lib/libgif.4.1.6.dylib
strip -x $PREFIX/lib/libjpeg.62.0.0.dylib
strip -x $PREFIX/lib/libpng.3.24.0.dylib
strip -x $PREFIX/lib/libtiff.3.dylib
strip -x $PREFIX/lib/libgeotiff.1.2.4.dylib
strip -x $PREFIX/lib/libjasper-1.701.1.0.0.dylib
strip -x $PREFIX/lib/libexpat.1.5.2.dylib
strip -x $PREFIX/lib/libfftw3.3.1.2.dylib
strip -x $PREFIX/lib/libgsl.0.9.0.dylib
strip -x $PREFIX/lib/libgslcblas.0.0.0.dylib
strip -x $PREFIX/lib/libpq.5.0.dylib

for LIBGRASS in datetime dbmibase dbmiclient dgl dig2 form gis gmath gproj I linkm rtree shape vask vect
do
	strip -x $PREFIX/lib/grass/libgrass_$LIBGRASS.6.3.0RC4.dylib
done

strip -x $PREFIX/share/qgis/python/qgis/core.so
strip -x $PREFIX/share/qgis/python/qgis/gui.so
strip -x $PREFIX/share/qgis/python/sip.so
for LIBPYQT4 in Qt QtCore QtGui QtNetwork QtSql QtSvg QtXml QtAssistant QtDesigner QtOpenGL QtScript QtTest
do
	strip -x $PREFIX/share/qgis/python/PyQt4/$LIBPYQT4.so
done
