#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThreadPool>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    private slots:
    void on_choosePicBtn_clicked();

    void on_chooseDirBtn_clicked();

    void on_exportBtn_clicked();

    private:
    void waterMarkThread();

    public:
    static const QString waterMarkVTop_;
    static const QString waterMarkVBottom_;
    static const QString waterMarkVCenter_;
    static const QString waterMarkHLeft_;
    static const QString waterMarkHRight_;
    static const QString waterMarkHCenter_;

    private:
    Ui::MainWindow *ui;
    QString picMarkPath_;
    QString sourceDicPath_;
    QString destDicPath_;
    QThreadPool *addWaterMarkpool_;
};

#endif // MAINWINDOW_H
