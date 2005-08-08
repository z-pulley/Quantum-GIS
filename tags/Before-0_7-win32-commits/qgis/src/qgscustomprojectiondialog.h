//
// C++ Interface: qgscustomprojectiondialog
//
// Description: 
//
//
// Author: Tim Sutton tim@linfiniti.com, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QGSCUSTOMPROJECTIONDIALOG_H
#define QGSCUSTOMPROJECTIONDIALOG_H

#include <qdir.h>
#include <qnetworkprotocol.h> 
#include <qgscustomprojectiondialogbase.uic.h>

/**
The custom projection widget is used to define the projection family, ellipsoid and paremters needed by proj4 to assemble a customised projection definition. The resulting projection will be store in an sqlite backend.

@author Tim Sutton
*/
class QgsCustomProjectionDialog : public QgsCustomProjectionDialogBase
{
Q_OBJECT
public:
    QgsCustomProjectionDialog(QWidget* parent , const char* name = "", WFlags fl=0);
    ~QgsCustomProjectionDialog();
    //a recursive function to make a directory and its ancestors
    bool makeDir(QDir &theQDir);
public slots:    
    void pbnCalculate_clicked();
    void pbnHelp_clicked();
    void pbnClose_clicked();
    void pbnDelete_clicked();
    //
    // Database navigation controles
    //
    long getRecordCount();
    void pbnFirst_clicked();
    void pbnPrevious_clicked();
    void pbnNext_clicked();
    void pbnLast_clicked();
    void pbnNew_clicked();
    void pbnSave_clicked();

    //
    // Contol population
    //
    /* These two methods will be deprecated
    void getProjList();
    void getEllipsoidList();
    */
    QString getProjectionFamilyName(QString theProjectionFamilyAcronym);
    QString getEllipsoidName(QString theEllipsoidAcronym);
    QString getProjectionFamilyAcronym(QString theProjectionFamilyName);
    QString getEllipsoidAcronym(QString theEllipsoidName);
private: 
    QString getProjFromParameters();
    QString getEllipseFromParameters();
    
        
    QString mCurrentRecordId;
    long mCurrentRecordLong;
    //the record previous to starting an insert operation
    //so that we can return to it if the record insert is aborted
    long mLastRecordLong;
    long mRecordCountLong;
    QString mQGisSettingsDir;
    const QString stringSQLSafe(const QString theSQL);

};

#endif
