#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QPainter>
#include <QRunnable>
#include <QThreadPool>
#include <thread>

class AddWaterMarkTask : public QRunnable {
    public:
    QString picMarkWEditText_;
    QString picMarkHEditText_;
    QString textMarkFontSizeEditText_;
    QString textMarkColorEditText_;
    QString waterMarkHBoxCurrentText_;
    QString waterMarkVBoxCurrentText_;

    QString imgPath_;
    QString textMark_;
    QString picMarkPath_;
    QString outputPath_;

    public:
    void run() {
        this->addWaterMarkByPath(imgPath_, textMark_, picMarkPath_, outputPath_);
    }

    //给某个路径的图片添加水印
    void addWaterMarkByPath(const QString &imgPath, const QString &text, const QString &picMarkPath, const QString &outputPath) {
        QPixmap pm(imgPath);
        if (pm.isNull()) {
            return;
        }

        QPainter painter(&pm);
        QPixmap picMark(picMarkPath);

        int picMarkW = picMarkWEditText_.toInt() == 0 ? picMark.width() : picMarkWEditText_.toInt();
        int picMarkH = picMarkHEditText_.toInt() == 0 ? picMark.height() : picMarkHEditText_.toInt();

        int fontSize = 100;
        if (textMarkFontSizeEditText_.toInt() == 0) {
            if (!picMark.isNull()) {
                fontSize = picMarkW;
            }
        } else {
            fontSize = textMarkFontSizeEditText_.toInt();
        }

        QColor fontColor(textMarkColorEditText_);
        if (!fontColor.isValid()) {
            fontColor = QColor("#ffffff");
        }

        //添加文字水印
        QFont font;
        font.setFamily("微软雅黑");
        font.setPixelSize(fontSize);
        painter.setFont(font);
        painter.setPen(fontColor);
        int padding = fontSize / 3;
        QFontMetrics fm(font);
        int textMarkW = fm.width(text);
        int textMarkH = fm.height();
        int textMarkX = 0;
        int textMarkY = 0;
        int picMarkX = 0;
        int picMarkY = 0;

        if (waterMarkHBoxCurrentText_ == MainWindow::waterMarkHLeft_) {
            picMarkX = padding;
            textMarkX = picMarkX + picMarkW + padding;
        } else if (waterMarkHBoxCurrentText_ == MainWindow::waterMarkHRight_) {
            textMarkX = pm.width() - textMarkW - padding;
            picMarkX = textMarkX - picMarkW - padding;
        } else if (waterMarkHBoxCurrentText_ == MainWindow::waterMarkHCenter_) {
            picMarkX = pm.width() / 2 - (picMarkW + padding + textMarkW) / 2;
            textMarkX = picMarkX + picMarkW + padding;
        }

        if (waterMarkVBoxCurrentText_ == MainWindow::waterMarkVTop_) {
            if (picMarkH >= textMarkH) {
                picMarkY = padding;
                textMarkY = picMarkY + picMarkH / 2 - textMarkH / 2;
            } else {
                textMarkY = padding;
                picMarkY = textMarkY + textMarkH / 2 - picMarkH / 2;
            }

        } else if (waterMarkVBoxCurrentText_ == MainWindow::waterMarkVBottom_) {
            if (picMarkH >= textMarkH) {
                picMarkY = pm.height() - padding - picMarkH;
                textMarkY = picMarkY + picMarkH / 2 - textMarkH / 2;
            } else {
                textMarkY = pm.height() - padding - textMarkH;
                picMarkY = textMarkY + textMarkH / 2 - picMarkH / 2;
            }
        } else if (waterMarkVBoxCurrentText_ == MainWindow::waterMarkVCenter_) {
            picMarkY = pm.height() / 2 - picMarkH / 2;
            textMarkY = pm.height() / 2 - textMarkH / 2;
        }

        painter.drawText(QRect(textMarkX, textMarkY, textMarkW, textMarkH), text);
        //添加图片水印
        if (!picMark.isNull()) {
            painter.drawPixmap(QRect(picMarkX, picMarkY, picMarkW, picMarkH), picMark);
        }

        QFile file(outputPath);
        file.open(QIODevice::WriteOnly);
        pm.save(&file);
        file.close();
    }
};

const QString MainWindow::waterMarkVTop_ = QStringLiteral("顶部");
const QString MainWindow::waterMarkVBottom_ = QStringLiteral("底部");
const QString MainWindow::waterMarkVCenter_ = QStringLiteral("垂直居中");
const QString MainWindow::waterMarkHLeft_ = QStringLiteral("左边");
const QString MainWindow::waterMarkHRight_ = QStringLiteral("右边");
const QString MainWindow::waterMarkHCenter_ = QStringLiteral("水平居中");

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    picMarkPath_ = QCoreApplication::applicationDirPath() + "/waterMark.png";
    ui->picPathLab->setText(picMarkPath_);

    ui->waterMarkVBox->addItem(MainWindow::waterMarkVTop_);
    ui->waterMarkVBox->addItem(MainWindow::waterMarkVCenter_);
    ui->waterMarkVBox->addItem(MainWindow::waterMarkVBottom_);
    ui->waterMarkHBox->addItem(MainWindow::waterMarkHLeft_);
    ui->waterMarkHBox->addItem(MainWindow::waterMarkHCenter_);
    ui->waterMarkHBox->addItem(MainWindow::waterMarkHRight_);
    ui->waterMarkVBox->setCurrentText(MainWindow::waterMarkVBottom_);
    ui->waterMarkHBox->setCurrentText(MainWindow::waterMarkHRight_);

    QRegExp regx("[0-9]+$");
    QValidator *validator = new QRegExpValidator(regx, this);
    ui->picMarkWEdit->setValidator(validator);
    ui->picMarkHEdit->setValidator(validator);
    ui->textMarkFontSizeEdit->setValidator(validator);

    addWaterMarkpool_ = new QThreadPool(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_choosePicBtn_clicked() {
    picMarkPath_ = QFileDialog::getOpenFileName();
    ui->picPathLab->setText(picMarkPath_);
}

void MainWindow::on_chooseDirBtn_clicked() {
    sourceDicPath_ = QFileDialog::getExistingDirectory();
    ui->chooseDirLab->setText(sourceDicPath_);
}

void MainWindow::on_exportBtn_clicked() {
    destDicPath_ = QFileDialog::getExistingDirectory();
    std::thread t(&MainWindow::waterMarkThread, this);
    t.detach();
}

void MainWindow::waterMarkThread() {
    QDir dir(sourceDicPath_);
    QFileInfoList fileInfos = dir.entryInfoList();
    int i = 0;
    for (QFileInfo fileInfo : fileInfos) {
        qDebug() << i << ":" << fileInfo.filePath();
        QString outputPath = destDicPath_ + "/" + fileInfo.fileName();
        AddWaterMarkTask *task = new AddWaterMarkTask();
        task->picMarkWEditText_ = ui->picMarkWEdit->text();
        task->picMarkHEditText_ = ui->picMarkHEdit->text();
        task->textMarkFontSizeEditText_ = ui->textMarkFontSizeEdit->text();
        task->textMarkColorEditText_ = ui->textMarkColorEdit->text();
        task->waterMarkHBoxCurrentText_ = ui->waterMarkHBox->currentText();
        task->waterMarkVBoxCurrentText_ = ui->waterMarkVBox->currentText();
        task->imgPath_ = fileInfo.filePath();
        task->textMark_ = ui->textMarkEdit->text();
        task->picMarkPath_ = picMarkPath_;
        task->outputPath_ = outputPath;
        addWaterMarkpool_->start(task);
        i++;
    }
}
