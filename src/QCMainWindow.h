#ifndef QCMAINWINDOW_H
#define QCMAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include "ui_QCMainWindow.h"

class QCMainWindow: public QMainWindow, private Ui::QCMainWindow {
    Q_OBJECT

public:
    QCMainWindow(QWidget *parent = 0);

protected:
    void changeEvent(QEvent *e);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    Ui::QCMainWindow *ui;
    QTime currentSongTimer;
    QTime currentNoteTimer;

private slots:
    void on_actionAbout_Qt_triggered();
    void on_actionQuit_USC_triggered();
    void on_actionAbout_triggered();
    void on_pushButton_BrowseCover_clicked();
    void on_pushButton_BrowseVideo_clicked();
    void on_pushButton_BrowseBackground_clicked();
    void on_pushButton_BrowseMP3_clicked();
    void on_pushButton_Stop_clicked();
    void on_pushButton_CopyToClipboard_clicked();
    void on_pushButton_PasteFromClipboard_clicked();
    void on_pushButton_Tap_released();
    void on_pushButton_Tap_pressed();
    void on_pushButton_Start_clicked();
    bool on_pushButton_SaveToFile_clicked();
};

#endif // QCMAINWINDOW_H
