/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
void QgsScanGeometries::setFileInfo( QString info)
{
    lblFileInfo->setText(info);
    progressBar1->setTotalSteps(0);
    progressBar1->setProgress(0);
}


void QgsScanGeometries::setStatus( int status )
{
    progressBar1->setProgress(progressBar1->progress() +2);
   
}
