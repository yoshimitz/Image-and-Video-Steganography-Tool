#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "stego.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupClickedEvents();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupClickedEvents()
{
    connect(ui->encodeSelectFileButton, &QPushButton::clicked, this, &MainWindow::onEncodeSelectFileButtonClicked);
    connect(ui->encodeSelectMediaButton, &QPushButton::clicked, this, &MainWindow::onEncodeSelectMediaButtonClicked);
    connect(ui->encodeGoButton, &QPushButton::clicked, this, &MainWindow::onEncodeGoButtonClicked);

    connect(ui->decodeSelectMediaButton, &QPushButton::clicked, this, &MainWindow::onDecodeSelectMediaButtonClicked);
    connect(ui->decodeGoButton, &QPushButton::clicked, this, &MainWindow::onDecodeGoButtonClicked);
}

void MainWindow::onEncodeSelectFileButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select File to Encode"), "/home/", tr("Files (*)"));

    if (!fileName.isNull())
    {
        ui->encodeSelectFileTextEdit->setText(fileName);
    }
}

void MainWindow::onEncodeSelectMediaButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Image/Video to Encode"), "/home/",
                                                    tr("Image/Video Files (*.png *.mp4)"));

    if (!fileName.isNull())
    {
        ui->encodeSelectMediaTextEdit->setText(fileName);
    }
}

void MainWindow::onEncodeGoButtonClicked()
{
    QMessageBox messageBox(this);
    messageBox.setWindowModality(Qt::ApplicationModal);
    messageBox.setWindowFlag(Qt::FramelessWindowHint);

    QString mediaPath = ui->encodeSelectMediaTextEdit->toPlainText();
    if (mediaPath.isEmpty())
    {
        messageBox.setText("Please select an image/video to encode.");
        messageBox.exec();
        return;
    }

    QString filePath = ui->encodeSelectFileTextEdit->toPlainText();
    if (filePath.isEmpty())
    {
        messageBox.setText("Please select a file to encode.");
        messageBox.exec();
        return;
    }

    bool bEncryption = ui->encodeEncryptionCheckbox->isChecked();
    std::string password = "";
    if (bEncryption)
    {
        QString passwordText = ui->encodePasswordLineEdit->text();
        if (passwordText.isEmpty())
        {
            messageBox.setText("Please enter a password to use encryption.");
            messageBox.exec();
            return;
        }

        password = passwordText.toStdString();
    }

    std::string stegoAlgo = ui->encodeStegoAlgoComboBox->currentText().toStdString();
    std::string edgeDetection = ui->encodeEdgeDetectionComboBox->currentText().toStdString();

    Stego stego(filePath.toStdString(), mediaPath.toStdString(), stegoAlgo, edgeDetection, bEncryption, password);

    StegoStatus status = StegoStatus::SUCCESS;

    if (bEncryption)
    {
        status = stego.EncryptFile();
    }

    if (status == StegoStatus::FILE_OPEN_FAILED)
    {
        messageBox.setText("Failed to open files for encryption.");
        messageBox.exec();
        return;
    }

    if (mediaPath.endsWith(".png"))
    {
         status = stego.EncodeImage();
    }
    else if (mediaPath.endsWith(".mp4"))
    {
        status = stego.EncodeVideo();
    }

    if (status == StegoStatus::FILE_TOO_LARGE)
    {
        messageBox.setText("File is too large to encode.");
        messageBox.exec();
        return;
    }
    else if (status == StegoStatus::VIDEO_OPEN_FAILED)
    {
        messageBox.setText("Failed to open video for encoding.");
        messageBox.exec();
        return;
    }
    else if (status == StegoStatus::SUCCESS)
    {
        messageBox.setText("File encoded in image!");
        messageBox.exec();
        return;
    }
}

void MainWindow::onDecodeSelectMediaButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Image/Video to Decode"), "/home/",
                                                    tr("Image/Video Files (*.png *.mp4)"));

    if (!fileName.isNull())
    {
        ui->decodeSelectMediaTextEdit->setText(fileName);
    }
}

void MainWindow::onDecodeGoButtonClicked()
{
    QMessageBox messageBox(this);
    messageBox.setWindowModality(Qt::ApplicationModal);
    messageBox.setWindowFlag(Qt::FramelessWindowHint);

    QString mediaPath = ui->decodeSelectMediaTextEdit->toPlainText();
    if (mediaPath.isEmpty())
    {
        messageBox.setText("Please select an image/video to decode.");
        messageBox.exec();
        return;
    }

    bool bEncryption = ui->decodeEncryptionCheckbox->isChecked();
    std::string password = "";
    if (bEncryption)
    {
        QString passwordText = ui->decodePasswordLineEdit->text();
        if (passwordText.isEmpty())
        {
            messageBox.setText("Please enter a password to use encryption.");
            messageBox.exec();
            return;
        }

        password = passwordText.toStdString();
    }

    StegoStatus status = StegoStatus::SUCCESS;
    Stego stego(mediaPath.toStdString(), bEncryption, password);

    if (mediaPath.endsWith(".png"))
    {
        status = stego.DecodeImage();
    }

    if (status == StegoStatus::INVALID_HEADER)
    {
        messageBox.setText("Unable to decode valid header.");
        messageBox.exec();
        return;
    }
    else if (status == StegoStatus::DATA_NOT_ENCRYPTED)
    {
        messageBox.setText("Password was given for decrypting but data is not encrypted.");
        messageBox.exec();
        return;
    }

    if (bEncryption)
    {
        status = stego.DecryptFile();
    }

    if (status == StegoStatus::FILE_OPEN_FAILED)
    {
        messageBox.setText("Failed to open files for decryption.");
        messageBox.exec();
        return;
    }
    else if (status == StegoStatus::DECRYPTION_FAILED)
    {
        messageBox.setText("Failed to decrypt file. Please check your password and try again.");
        messageBox.exec();
        return;
    }
    else if (status == StegoStatus::SUCCESS)
    {
        messageBox.setText("File Decoded!");
        messageBox.exec();
        return;
    }
}

