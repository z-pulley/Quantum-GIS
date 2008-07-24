/***************************************************************************
     qgsgrassshell.h

    Date                 : Sun Sep 16 12:06:10 AKDT 2007
    Copyright            : (C) 2007 by Radim Blazek 
    Email                : blazek at itc.it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QByteArray>
#include <QProcess>
#include <QMessageBox>
#include <QSocketNotifier>
#include <QPointer>
#include <QTextEdit>

class QProgressBar;



extern "C" {
#include <stdio.h>
}

/* QgsGrassShellBase is simple pseudo terminal implementing 
 * very limited subset of VT100. OTOH it parses GUI format of 
 * messages printed by GRASS modules and adds icons for 
 * WARNING and ERROR and it also resets progress bar if a module
 * uses G_percent() function.
 *
 * On UNIX pseudo terminal is represented by a pair of files 
 * called master and slave which are connected. 
 * On LINUX the files are called /dev/ptmx (master) 
 * and /dev/pts/<number> (slave). The application emulating terminal
 * is connected master and the program running in terminal
 * (usually shell like bash,tcsh,sh) is connected to slave.
 * 
 * There is no support for pseudo terminals in Qt. Everything 
 * must be written using relatively low level C functions 
 * 
 * Input (LINUX):
 * keyboard -> X -> Qt -> QgsGrassShellBase::keyPressEvent()
 * -> /dev/ptmx -> /dev/pts/<number> -> shell
 *
 * Output (LINUX):
 * shell -> /dev/pts/<number> -> /dev/ptmx 
 * -> QgsGrassShell::readStdout() -> QByteArray -> 
 * -> QgsGrassShell::printStdout() -> QTextEdit
 *
 * Reading/printing of shell output is bease on two prerrequisites:
 *   1) Lines printed by shell are not endless
 *   2) GRASS GUI messages are always available complete on output
 * thus we read everything on output to buffer in cycle untill 
 * we reach either EOF (no more data available) or NL (new line).
 * Then the data from buffer are printed to output QTextEdit.
 * Control characters, control sequences and GRASS GUI messages
 * are transformed to appropriate output written to QTextEdit.
 */

class QgsGrassShell: public QTextEdit
{
    Q_OBJECT;

public:
    QgsGrassShell (QWidget * parent = 0, const char * name = 0 );
    ~QgsGrassShell();

    // Modes 
    enum Mode
    {
	Origin = 0,
	Wrap,
	Insert, 
	Screen,
	Cursor,
	NewLine,
	ModeCount
    };

    // Graphic Rendition
    enum Rendition
    {
        Bold = 0,
        Underline,
        Blink,
        Reverse,
        RenditionCount
    };

    // Pressed key 
    enum KeyDown
    {
	DownControl = 0,
	DownShift,
	DownAlt, 
	DownMeta,
	KeyDownCount
    };

    // Print maximum form standard input buffer
    void printStdout();

    // Print maximum form standard error buffer
    void printStderr();

    // Set / reset mode
    void setMode ( int m ) { mMode[m] = true; }
    void resetMode ( int m ) { mMode[m] = false; }

    // Insert text on current cursor position according to Insert mode
    void insert ( QString );

    // Start new line
    void newLine();

    // Erase cursor highlight (space)
    void eraseCursor(void);

    // Highlite cursor - delete this? [TS]
   //void showCursor(void);

    // Reset pseudoterminal size
    void resizeTerminal ();

    // Remove empty (or with whitespaces) paragraphs
    void removeEmptyParagraphs();

    // Reset cursor to current position
    //void setCursorPosition();
    
public slots:

  void focusInEvent ( QFocusEvent * event );
  void focusOutEvent ( QFocusEvent * event );
  // filter keypresses from going to main app
  bool eventFilter( QObject *object, QEvent *event );
  void keyPressEvent ( QKeyEvent * e );
  void keyReleaseEvent ( QKeyEvent * e );
  void mousePressEvent(QMouseEvent* e);
  void resizeEvent ( QResizeEvent * );

  void readStdout(int socket);
  void readStderr();
signals:

private:
    //QProcess *mProcess;
    // Master file descriptor
    int mFdMaster;

    /**
     *    * The socket descriptors for stdout.
     *       */
    int out[2];

    QSocketNotifier *mOutNotifier;

    // Buffer for data read form shell stdout
    QByteArray mStdoutBuffer; 

    // Buffer for data read form shell stderr
    QByteArray mStderrBuffer; 

    // Shell process PID
    int mPid;

    // Modes
    bool mMode[ModeCount];

    // Flag for space if it was added at the end of paragraph to 
    // visualize the cursor
    bool mCursorSpace; 

    // Current cursor position
    int mBlockNo;
    int mPosition;

    // Progress bar
    QPointer <QProgressBar> mpProgressBar;


    // Rendition
    bool mRendition[RenditionCount];

    // Pressed keys
    bool mKeyDown[KeyDownCount];


    // Create new line in next insert
    // This must be used because QTextEdit will remove all empty
    // paragraphs
    bool mNewLine;

    // Is it successfuly opened
    bool mValid;

    // Horizontal tabulator stop
    std::vector<bool> mTabStop;

    // How many lines to skip, used to skip output of first commands sent to shell
    int mSkipLines; 
    
};



