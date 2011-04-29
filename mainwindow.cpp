#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsPixmapItem>
#include <QTransform>
#include <QMatrix>
#include <QGraphicsScene>
#include <QImage>
#include <QImageReader>

#include <QFile>
#include <QScrollBar>
#include <QLabel>
#include <QTextEdit>

#include <QDesktopWidget>
#include <QFileDialog>
#include <QDir>

// prefer ISO-standard type names when available..
#include <stdint.h>

#include "FileType.h"
#include "IffIlbm.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
	m_pFile(nullptr),
	m_pScene(NULL),
	m_pCurImage(NULL),
	//m_szCurImagePath(),
	m_szCurPath(),
	m_ImageList(),
	m_iCurImageIx(-1),
	m_bMaximized(false)
{
    ui->setupUi(this);
	m_pScene = new QGraphicsScene(this);
	ui->graphicsView->setScene(m_pScene);

	m_szBaseTitle = this->windowTitle();
	
	connect(this, SIGNAL(FileSelection(QString)), SLOT(FileChanged(QString)));
	connect(this, SIGNAL(PathRefresh(QString)), SLOT(PathFileRefresh(QString)));

	// note: set before showing image so window-resize will know this..
	showMaximized();
	m_bMaximized = true;
	
	// if file(s) given in command line,
	// keep as list for browsing
	QStringList vCmdLine = QApplication::arguments();
	if (vCmdLine.size() > 1)
	{
		// remove app name
		vCmdLine.removeAt(0);
		
		if (vCmdLine.size() > 1)
		{
			// keep image list
			// as "playlist" when more than one file given,
			// selecting file manually resets the list also
			m_ImageList = vCmdLine;
		}
		
		emit FileSelection(vCmdLine[0]);

		// for image browsing/switching
		emit PathRefresh(vCmdLine[0]);
	}
}

MainWindow::~MainWindow()
{
	if (m_pCurImage != NULL)
	{
		delete m_pCurImage;
	}
	if (m_pFile != nullptr)
	{
		m_pFile->close();
		delete m_pFile;
	}
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Up)
	{
		// meh.. try faking it
		QScrollBar *vscroll = ui->graphicsView->verticalScrollBar();
		vscroll->triggerAction(QScrollBar::SliderSingleStepSub);
		
		// scrolls entire widget (not just viewport)
		//ui->graphicsView->scroll(0, 10);
		//scroll(0, 10);
		
		// not necessary: starts as accepted
		//e->accept();
		return;
	}
	else if (e->key() == Qt::Key_Down)
	{
		// meh.. try faking it
		QScrollBar *vscroll = ui->graphicsView->verticalScrollBar();
		vscroll->triggerAction(QScrollBar::SliderSingleStepAdd);
		
		// ok, this gives us both current scaling
		// and current translation (scrolled position)
		// of the view
		//QTransform vptr = ui->graphicsView->viewportTransform();
		// translate() seems to want absolute coordinates 
		// instead of relative delta??
		//ui->graphicsView->translate(0, 100);
		// scrolls entire widget (not just viewport)
		//ui->graphicsView->scroll(0, -10);
		//scroll(0, -10);
		// drawing isn't correct but at least affects actual image..
		//ui->graphicsView->viewport()->scroll(0, -10);
		return;
	}
	
	// pass to base-class
	QMainWindow::keyPressEvent(e);
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Up
		|| e->key() == Qt::Key_Down)
	{
		// not necessary: starts as accepted
		//e->accept();
		return;
	}
	
	// ignore or pass to base-class?
	//e->ignore();
	QMainWindow::keyPressEvent(e);
}

void MainWindow::ResizeToCurrent()
{
	if (m_pCurImage == NULL)
	{
		return;
	}
	
	// resizing of window to view (picture),
	// only on "normal" (windowed) more
	if (isFullScreen() == false
		&& isMaximized() == false)
	{
		// check size for resize of main window:
		// check size of frame so that we can adjust and show image fully without borders
		// note: at program loading, frame and window have equal size
		// -> our picture doesn't fit entirely after the count&loading..
		//
		//QSize frsize = frameSize();
		//QSize tsize = size();
		resize(m_pCurImage->size() + (frameSize() - size()));
		
		// check this..
		// nope, reduced to few pixels at start..
		//ui->graphicsView->fitInView(m_pCurImage->rect(), Qt::KeepAspectRatio);
	}
	
	// also repositioning at center of image in case large picture
	QRect imagesize = m_pCurImage->rect();
	ui->graphicsView->centerOn(imagesize.width()/2, imagesize.height()/2);
}

// rescale image when user presses "rescale" (reset)
// to fit image into view/return to original size
//
void MainWindow::RescaleImage()
{
	if (m_pCurImage == NULL)
	{
		return;
	}
	
	// TODO: actually, "fit to width" of window might be more useful..
	// at least scaling down of big pics?
	
	// reset image-view scaling also
	ui->graphicsView->resetTransform();

	qreal rScale = 1.0;
	QRect imagesize = m_pCurImage->rect();
	QRect maxsize = rect(); // window size

	if (isFullScreen() == true)
	{
		// quick hack to downscale when image is larger than screen..
		maxsize = QApplication::desktop()->screenGeometry();
	}
	
	// test, not exactly wanted, see possibilities though..
	//ui->graphicsView->fitInView(maxsize, Qt::KeepAspectRatio);
	
	// also repositioning at center of image in case large picture
	ui->graphicsView->centerOn(imagesize.width()/2, imagesize.height()/2);

	if (imagesize.width() > maxsize.width())
	{
		// TODO: better way to scale according to actual view-area (take framesize into account),
		// also height and scroll bars..
		rScale = (qreal)maxsize.width() / (qreal)imagesize.width();
	}
	else if (imagesize.height() > maxsize.height())
	{
		rScale = (qreal)maxsize.height() / (qreal)imagesize.height();
	}
	ui->graphicsView->scale(rScale, rScale);
}

// check that format of given file is supported
// before loading,
// also check if we need our own loading routines..
//
bool MainWindow::CheckFormat(const QString &szFile, QString &szFormat)
{
	// what can we load..
	QList<QByteArray> lstFormats = QImageReader::supportedImageFormats();
	
	if (m_pFile != nullptr)
	{
		m_pFile->close();
		delete m_pFile;
	}

	m_pFile = new QFile(szFile, this);
	if (m_pFile == nullptr)
	{
		return false;
	}
	if (m_pFile->open(QIODevice::ReadOnly) == false)
	{
		delete m_pFile;
		m_pFile = nullptr;
		return false;
	}

	// map entire file into view
	qint64 iSize = m_pFile->size();
	uchar *pView = m_pFile->map(0, iSize);
	if (pView == NULL
		|| iSize == 0)
	{
		delete m_pFile;
		m_pFile = nullptr;
		return false;
	}

	//tFormatType enFileType = FORMAT_UNKNOWN;
	
	// get identifier at start
	//uint32_t ulFirstFour = ((pView[3] << 24) + (pView[2] << 16) + (pView[1] << 8) + pView[0]);
	
	CFileType Type;
	tHeaderType enFormat = Type.FileTypeFromHeader(pView, iSize);
	
	// check if format is supported by Qt
	bool bLoaderExists = false;
	switch (enFormat)
	{
	case HEADERTYPE_ILBM:
		szFormat = "ilbm";
		break;
	case HEADERTYPE_JPEG:
		szFormat = "jpeg";
		break;
	case HEADERTYPE_PNG:
		szFormat = "png";
		break;
	case HEADERTYPE_GIF:
		szFormat = "gif";
		break;
	case HEADERTYPE_TIFF:
		szFormat = "tiff";
		break;
	}
	
	// if format handling supported -> let Qt do loading
	//
	bLoaderExists = lstFormats.contains(szFormat.toAscii());
	if (bLoaderExists == true)
	{
		return true;
	}

	// no loader, see if we have our own..
	if (bLoaderExists == false
		&& enFormat == HEADERTYPE_ILBM)
	{
		// load&parse ILBM
		CIffIlbm IffIlbm;
		//IffIlbm.ParseFile();
	}
	
	// not yet completed..
	// close file
	m_pFile->close();
	delete m_pFile;
	m_pFile = nullptr;
	return false;
}

void MainWindow::FileChanged(QString szFile)
{
	setWindowTitle(m_szBaseTitle);
	
	// destroy old (if any)
	if (m_pCurImage != NULL)
	{
		delete m_pCurImage;
	}

	// clear previously loaded images
	if (m_pScene != NULL)
	{
		// clear previous pictures from scene list
		m_pScene->clear();
		
		// reset scaling level when image changes
		ui->graphicsView->resetTransform();
		
		// TODO: these also? is there a case to need these?
		//ui->graphicsView->resetCachedContent();
		//ui->graphicsView->resetMatrix();
	}

	bool bRet = false;
	QString szFormat;
	if (CheckFormat(szFile, szFormat) == false)
	{
		// load failure or not supported format?
		return;
	}

	// keep for later
    m_pCurImage = new QImage();
	
	// if format is supported by Qt -> use loader
	// otherwise -> use our own loader
	//
	if (szFormat.length() > 0
		&& m_pFile != nullptr)
	{
		bRet = m_pCurImage->load(m_pFile, szFormat.toAscii());
	}
	else
	{
		bRet = m_pCurImage->load(szFile);
	}
    if (bRet == false)
    {
		// failed loading: cleanup
		delete m_pCurImage;
		m_pCurImage = NULL;
		return;
	}

	setWindowTitle(m_szBaseTitle + " - " + szFile);
	
	// pixmap from image and set to scene,
	// resize scene to image
	m_pScene->addPixmap(QPixmap::fromImage((*m_pCurImage)));
	m_pScene->setSceneRect(m_pCurImage->rect());

	// resize main window to fit current image
	ResizeToCurrent();
}

void MainWindow::PathFileRefresh(QString szFilePath)
{
	// reverse find last path separator,
	// strip filename from path
	// and check for supported files in path
	szFilePath.replace('\\', "/");
	int iPos = szFilePath.lastIndexOf('/');
	if (iPos == -1)
	{
		// does not have path separator as expected -> nothing to do
		setWindowTitle(m_szBaseTitle);
		return;
	}

	QString szFile = szFilePath.right(szFilePath.length() - iPos -1);

	// quick hack to show current file
	QString szTitle = m_szBaseTitle + " - " + szFile;
	setWindowTitle(szTitle);
	
	if (m_ImageList.length() < 1)
	{
		m_szCurPath = szFilePath.left(iPos + 1);

		// TODO: filter by supported image-codecs..
		// see: QImageReader::supportedImageFormats()
		
		QDir Dir(m_szCurPath);
		m_ImageList = Dir.entryList(QDir::Files | QDir::NoDotAndDotDot, 
									QDir::Name | QDir::IgnoreCase);
	}
	
	m_iCurImageIx = m_ImageList.indexOf(szFile);
}

void MainWindow::on_actionFile_triggered()
{
    // open dialog for user to select file

    QString szFileName = QFileDialog::getOpenFileName(this, tr("Open file"));
    if (szFileName == NULL)
    {
		// user cancelled?
		return;
    }

	// reset "playlist" of files
	// when user selects a file,
	// path may be changed by user..
	m_ImageList.clear();
	
	// open selected image
	emit FileSelection(szFileName);
	
	// for image browsing/switching
	emit PathRefresh(szFileName);
}

void MainWindow::on_actionFullscreen_triggered(bool checked)
{
	// "fullscreen" checked
    if (checked == true)
    {
		m_bMaximized = isMaximized();
		showFullScreen();
    }
	else
	{
		// case: if it was maximized before fullscreen,
		// return to maximized..
		if (m_bMaximized == true)
		{
			showMaximized();
		}
		else
		{
			showNormal();
		}
	}

	// resize main window to fit current image:
	// in case different size image was selected while in fullscreen mode
	ResizeToCurrent();
	
	// reset image-view scaling on screen mode switch
	ui->graphicsView->resetTransform();
}

void MainWindow::on_actionZoom_in_triggered()
{
    if (m_pScene == NULL
		|| m_pCurImage == NULL)
    {
		return;
    }
	
	ui->graphicsView->scale(1.25, 1.25);
}

void MainWindow::on_actionZoom_out_triggered()
{
    if (m_pScene == NULL
		|| m_pCurImage == NULL)
    {
		return;
    }
    
	ui->graphicsView->scale(0.75, 0.75);
}

void MainWindow::on_actionReset_zoom_triggered()
{
	// resize main window to fit current image:
	// use original size of image
	ResizeToCurrent();
	RescaleImage();
}

void MainWindow::on_actionNext_triggered()
{
	if (m_ImageList.length() <= 1)
	{
		// only one file in list..?
		return;
	}
	
	if (m_iCurImageIx < (m_ImageList.length() -1))
	{
		++m_iCurImageIx;
	}
	else
	{
		// wraparound
		m_iCurImageIx = 0;
	}

	QString szNew = m_szCurPath + m_ImageList[m_iCurImageIx];
	emit FileSelection(szNew);
	emit PathRefresh(szNew);
}

void MainWindow::on_actionPrevious_triggered()
{
	if (m_ImageList.length() <= 1)
	{
		// only one file in list..?
		return;
	}

	if (m_iCurImageIx > 0)
	{
		--m_iCurImageIx;
	}
	else
	{
		// wraparound
		m_iCurImageIx = (m_ImageList.length() -1);
	}

	QString szNew = m_szCurPath + m_ImageList[m_iCurImageIx];
	emit FileSelection(szNew);
	emit PathRefresh(szNew);
}

void MainWindow::on_actionAbout_triggered()
{
	// show about-dialog

	on_actionHelp_triggered();
/*	
	QLabel *pTxt = new QLabel(" qPicView by Ilkka Prusi 2011 ", this);
	pTxt->setWindowFlags(Qt::Window); //or Qt::Tool, Qt::Dialog if you like
	pTxt->show();
*/
}

void MainWindow::on_actionHelp_triggered()
{
    // generic help
	
	QTextEdit *pTxt = new QTextEdit(this);
	pTxt->setWindowFlags(Qt::Window); //or Qt::Tool, Qt::Dialog if you like
	pTxt->setReadOnly(true);
	pTxt->append("qPicView by Ilkka Prusi 2011");
	pTxt->append("");
	pTxt->append("This program is free to use and distribute. No warranties of any kind.");
	pTxt->append("Program uses Qt 4.7.1 under LGPL v. 2.1");
	pTxt->append("");
	pTxt->append("Keyboard shortcuts:");
	pTxt->append("");
	pTxt->append("* = resize/fit");
	pTxt->append("+ = zoom in");
	pTxt->append("- = zoom out");
	pTxt->append("Left = previous");
	pTxt->append("Right = next");
	pTxt->append("Up/Down = scroll");
	pTxt->append("");
	pTxt->append("F = open file");
	pTxt->append("F1 = help (this)");
	pTxt->append("F11 = fullscreen toggle");
	pTxt->append("Esc = close");
	pTxt->append("");
	pTxt->append("Tip: set as default program :)");
	pTxt->show();
}

