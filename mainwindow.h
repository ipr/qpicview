#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>

#include <QStringList>

// prefer ISO-standard type names when available..
#include <stdint.h>


// changed: prefer fwd. decl. here..
class QGraphicsScene;
class QImage;
class QFile;


namespace Ui {
    class MainWindow;
}

// type of file data
// according to header in file
//
typedef enum tFormatType
{
	FORMAT_UNKNOWN = 0,
	
	FORMAT_ILBM, // IFF-ILBM (Amiga)
	
	FORMAT_JPEG,
	FORMAT_PNG,
	FORMAT_GIF,
	FORMAT_TIFF
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *e);
	
	void ResizeToCurrent();
	void RescaleImage();
	
	bool CheckFormat(const QString &szFile, QString &szFormat);
	
private:
    Ui::MainWindow *ui;
	QFile *m_pFile;
	QGraphicsScene *m_pScene;
	QImage *m_pCurImage;
	//QString m_szCurImagePath;
	QString m_szBaseTitle;
	QString m_szCurPath;
	QStringList m_ImageList;
	int m_iCurImageIx;
	
	// if was maximized when entering fullscreen
	bool m_bMaximized;

signals:
	void FileSelection(QString);
	void PathRefresh(QString);
	
public slots:
    void FileChanged(QString szFile);
    void PathFileRefresh(QString szFilePath);
	
private slots:
    void on_actionHelp_triggered();
    void on_actionAbout_triggered();
    void on_actionPrevious_triggered();
    void on_actionNext_triggered();
    void on_actionReset_zoom_triggered();
    void on_actionZoom_out_triggered();
    void on_actionZoom_in_triggered();
    void on_actionFullscreen_triggered(bool checked);
    void on_actionFile_triggered();
};

#endif // MAINWINDOW_H
