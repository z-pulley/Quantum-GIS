TEMPLATE = lib
TARGET=libqgis
INCLUDEPATH += . $(GDAL)\include \
                $(POSTGRESQL)\src\interfaces\libpq \
                $(POSTGRESQL)\src\include
LIBS += $(GDAL)\lib\gdal_i.lib
CONFIG += qt thread rtti
DLLDESTDIR= ..\..\src\lib\qgis
# Input
SOURCES += moc_qgisapp.cpp \
moc_qgisappbase.cpp \
moc_qgisinterface.cpp \ 
moc_qgsabout.cpp \
moc_qgsattributetable.cpp \
moc_qgsattributetablebase.cpp \
moc_qgscontcoldialog.cpp \
moc_qgscontcoldialogbase.cpp \
moc_qgsdbsourceselect.cpp \
moc_qgsdbsourceselectbase.cpp \
moc_qgsdlgvectorlayerproperties.cpp \
moc_qgsdlgvectorlayerpropertiesbase.cpp \
moc_qgsgramadialog.cpp \
moc_qgsgramadialogbase.cpp \
moc_qgsgramaextensionwidget.cpp \
moc_qgsgrasydialog.cpp \
moc_qgsgrasydialogbase.cpp \
moc_qgshelpviewer.cpp \
moc_qgshelpviewerbase.cpp \
moc_qgsidentifyresults.cpp \
moc_qgsidentifyresultsbase.cpp \
moc_qgslabeldialog.cpp \
moc_qgslabeldialogbase.cpp \
moc_qgslegend.cpp \
moc_qgslegenditembase.cpp \
moc_qgslinestyledialog.cpp \
moc_qgslinestyledialogbase.cpp \
moc_qgsmapcanvas.cpp \
moc_qgsmaplayer.cpp \
moc_qgsmaplayerinterface.cpp \
moc_qgsmaplayerregistry.cpp \
moc_qgsmapserverexport.cpp \
moc_qgsmapserverexportbase.cpp \
moc_qgsmarkerdialog.cpp \
moc_qgsmarkerdialogbase.cpp \
moc_qgsmessageviewer.cpp \
moc_qgsnewconnection.cpp \
moc_qgsnewconnectionbase.cpp \
moc_qgsoptions.cpp \
moc_qgsoptionsbase.cpp \
moc_qgspatterndialog.cpp \
moc_qgspatterndialogbase.cpp \
moc_qgspluginmanager.cpp \
moc_qgspluginmanagerbase.cpp \
moc_qgsprojectproperties.cpp \
moc_qgsprojectpropertiesbase.cpp \
moc_qgsrasterlayer.cpp \
moc_qgsrasterlayerproperties.cpp \
moc_qgsrasterlayerpropertiesbase.cpp \
moc_qgssimadialog.cpp \
moc_qgssimadialogbase.cpp \
moc_qgssisydialog.cpp \
moc_qgssisydialogbase.cpp \
moc_qgsuvaldialog.cpp \
moc_qgsuvaldialogbase.cpp \
moc_qgsludialogbase.cpp \
moc_qgsludialog.cpp \
moc_qgsvectorlayer.cpp \
moc_splashscreen.cpp \
moc_qgsuvalmadialog.cpp \
moc_qgsuvalmadialogbase.cpp \
qgisapp.cpp \
qgisappbase.cpp \
qgisiface.cpp \
qgisinterface.cpp \
qgsabout.cpp \
qgsacetateobject.cpp \
qgsacetaterectangle.cpp \
qgsattributetable.cpp \
qgsattributetablebase.cpp \
qgsattributetabledisplay.cpp \
qgscontcoldialog.cpp \
qgscontcoldialogbase.cpp \
qgscontinuouscolrenderer.cpp \
qgscoordinatetransform.cpp \
qgscustomsymbol.cpp \
qgsdatasource.cpp \
qgsdbsourceselect.cpp \
qgsdbsourceselectbase.cpp \
qgsdlgvectorlayerproperties.cpp \
qgsdlgvectorlayerpropertiesbase.cpp \
qgsfeature.cpp \
qgsfeatureattribute.cpp \
qgsfield.cpp \
qgsgraduatedmarenderer.cpp \
qgsgraduatedsymrenderer.cpp \
qgsgramadialog.cpp \
qgsgramadialogbase.cpp \
qgsgramaextensionwidget.cpp \
qgsgrasydialog.cpp \
qgsgrasydialogbase.cpp \
qgshelpviewer.cpp \
qgshelpviewerbase.cpp \
qgsidentifyresults.cpp \
qgsidentifyresultsbase.cpp \
qgslabel.cpp \
qgslabelattributes.cpp \
qgslabeldialog.cpp \
qgslabeldialogbase.cpp \
qgslegend.cpp \
qgslegenditem.cpp \
qgslegenditembase.cpp \
qgslinestyledialog.cpp \
qgslinestyledialogbase.cpp \
qgslinesymbol.cpp \
qgsmapcanvas.cpp \
qgsmaplayer.cpp \
qgsmaplayerregistry.cpp \
qgsmapserverexport.cpp \
qgsmapserverexportbase.cpp \
qgsmarkerdialog.cpp \
qgsmarkerdialogbase.cpp \
qgsmarkersymbol.cpp \
qgsmessageviewer.cpp \
qgsnewconnection.cpp \
qgsnewconnectionbase.cpp \
qgsoptions.cpp \
qgsoptionsbase.cpp \
qgspatterndialog.cpp \
qgspatterndialogbase.cpp \
qgspluginitem.cpp \
qgspluginmanager.cpp \
qgspluginmanagerbase.cpp \
qgspluginmetadata.cpp \
qgspluginregistry.cpp \
qgspoint.cpp \
qgspolygonsymbol.cpp \
qgsprojectio.cpp \
qgsprojectproperties.cpp \
qgsprojectpropertiesbase.cpp \
qgsprovidermetadata.cpp \
qgsproviderregistry.cpp \
qgsrangerenderitem.cpp \
qgsrasterlayer.cpp \
qgsrasterlayerproperties.cpp \
qgsrasterlayerpropertiesbase.cpp \
qgsrect.cpp \
qgsrenderitem.cpp \
qgsscalecalculator.cpp \
qgssimadialog.cpp \
qgssimadialogbase.cpp \
qgssimarenderer.cpp \
qgssinglesymrenderer.cpp \
qgssisydialog.cpp \
qgssisydialogbase.cpp \
qgssvgcache.cpp \
qgssymbol.cpp \
qgssymbologyutils.cpp \
qgsuniquevalrenderer.cpp \
qgsuvaldialog.cpp \
qgsuvaldialogbase.cpp \
qgsvectorfilewriter.cpp \
qgsludialogbase.cpp \
qgsludialog.cpp \
qgsvectorlayer.cpp \
qgsuvalmadialog.cpp \
qgsuvalmadialogbase.cpp \
qgsuvalmarenderer.cpp \
qgsgeomtypedialog.cpp \
qgsgeomtypedialogbase.cpp \
moc_qgsgeomtypedialogbase.cpp 
