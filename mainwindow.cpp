#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "stego.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTimer>
#include <QtQuick/QQuickView>
#include <QUrl>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , apiClient(StegoApiClient())
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

    connect(ui->webLoginPushButton, &QPushButton::clicked, this, &MainWindow::onWebLoginPushButtonClicked);
    connect(ui->webRegisterPushButton, &QPushButton::clicked, this, &MainWindow::onWebRegisterPushButtonClicked);

    connect(ui->mediaListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onListItemDoubleClicked);
}

void MainWindow::onEncodeSelectFileButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select File to Encode"), QDir::homePath(), tr("Files (*)"));

    if (!fileName.isNull())
    {
        ui->encodeSelectFileTextEdit->setText(fileName);
    }
}

void MainWindow::onEncodeSelectMediaButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Image/Video to Encode"), QDir::homePath(),
                                                    tr("Image/Video Files (*.png *.mkv)"));

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

    if (ui->uploadCheckBox->isChecked() && apiClient.GetApiToken() == "")
    {
        messageBox.setText("Upload is selected but user is not logged in. Please log in on the web tab and try again.");
        messageBox.exec();
        return;
    }

    if (ui->uploadCheckBox->isChecked() && std::filesystem::file_size(mediaPath.toStdString()) > 200000000)
    {
        messageBox.setText("Media is too large to upload to web. (200MB limit)");
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

    QCoreApplication::processEvents();

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

    QProgressDialog *progressDialog = new QProgressDialog("Processing... (this may take some time if encoding in a video)", "Cancel", 0, 0, this);
    progressDialog->setWindowModality(Qt::ApplicationModal);
    progressDialog->setWindowFlag(Qt::FramelessWindowHint);
    progressDialog->setCancelButton(nullptr);
    progressDialog->setMinimumDuration(0);
    progressDialog->setValue(0);
    progressDialog->show();
    QCoreApplication::processEvents();

    if (mediaPath.endsWith(".png"))
    {
         status = stego.EncodeImage();
    }
    else if (mediaPath.endsWith(".mkv"))
    {
        status = stego.EncodeVideo();
    }

    QCoreApplication::processEvents();

    if (ui->uploadCheckBox->isChecked() && status == StegoStatus::SUCCESS)
    {
        std::string mediaName = std::filesystem::path(mediaPath.toStdString()).filename().string();
        std::string stegoMediaPath = "stego_media/" + mediaName;
        if (std::filesystem::file_size(stegoMediaPath) > 200000000)
        {
            QMetaObject::invokeMethod(progressDialog, "reset");
            delete progressDialog;
            messageBox.setText("File encoded in image/video but is too large to upload to web.");
            messageBox.exec();
            return;
        }

        progressDialog->setLabelText("Uploading...");
        std::ifstream file(stegoMediaPath, std::ios_base::binary);
        long result = apiClient.Upload(mediaName, file);
        if (result != 201)
        {
            file.close();
            QMetaObject::invokeMethod(progressDialog, "reset");
            delete progressDialog;
            messageBox.setText("File encoded in image/video but failed to upload to web.");
            messageBox.exec();
            return;
        }

        file.close();

        apiClient.GetList();

        ui->mediaListWidget->clear();
        ui->mediaListWidget->addItems(apiClient.GetMediaList());
    }

    QMetaObject::invokeMethod(progressDialog, "reset");
    delete progressDialog;

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
    else if (status == StegoStatus::VIDEO_REENCODING_FAILED)
    {
        messageBox.setText("Video failed to re-encode after steganographic changes.");
        messageBox.exec();
        return;
    }
    else if (status == StegoStatus::SUCCESS)
    {
        messageBox.setText("File encoded in image/video!");
        messageBox.exec();
        return;
    }
}

void MainWindow::onDecodeSelectMediaButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Image/Video to Decode"), QDir::homePath(),
                                                    tr("Image/Video Files (*.png *.mkv)"));

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

    QProgressDialog *progressDialog = new QProgressDialog("Processing...", "Cancel", 0, 0, this);
    progressDialog->setWindowModality(Qt::ApplicationModal);
    progressDialog->setWindowFlag(Qt::FramelessWindowHint);
    progressDialog->setCancelButton(nullptr);
    progressDialog->setMinimumDuration(0);
    progressDialog->setValue(0);
    progressDialog->show();
    QCoreApplication::processEvents();

    if (mediaPath.endsWith(".png"))
    {
        status = stego.DecodeImage();
    }
    else if (mediaPath.endsWith(".mkv"))
    {
        status = stego.DecodeVideo();
    }

    QMetaObject::invokeMethod(progressDialog, "reset");
    delete progressDialog;

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
    else if (status == StegoStatus::INVALID_MEDIA)
    {
        messageBox.setText("Failed to get file from media. May not be steganographic image.");
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

void MainWindow::onWebLoginPushButtonClicked()
{
    QMessageBox messageBox(this);
    messageBox.setWindowModality(Qt::ApplicationModal);
    messageBox.setWindowFlag(Qt::FramelessWindowHint);

    QString emailText = ui->webEmailLineEdit->text();
    if (emailText.isEmpty())
    {
        messageBox.setText("Please enter an email to log in.");
        messageBox.exec();
        return;
    }

    QString passwordText = ui->webPasswordLineEdit->text();
    if (passwordText.isEmpty())
    {
        messageBox.setText("Please enter a password to log in.");
        messageBox.exec();
        return;
    }

    long statusCode = apiClient.Login(emailText.toStdString(), passwordText.toStdString());

    if (statusCode != 200)
    {
        messageBox.setText("Failed to log in. Please check that your email and password are correct.");
        messageBox.exec();
        return;
    }

    int status = apiClient.GetList();
    if (status != 200)
    {
        messageBox.setText("Failed to get media list. Please check your internet connection and try again.");
        messageBox.exec();
        return;
    }

    ui->mediaListWidget->addItems(apiClient.GetMediaList());
    ui->stackedWidget->setCurrentWidget(ui->page2);
}

void MainWindow::onWebRegisterPushButtonClicked()
{
    QDesktopServices::openUrl(QUrl("https://steganographywebapp20240927173619.azurewebsites.net/Identity/Account/Register", QUrl::TolerantMode));
}

void MainWindow::onListItemDoubleClicked(QListWidgetItem* item)
{
    QProgressDialog *progressDialog = new QProgressDialog("Downloading...", "Cancel", 0, 0, this);
    progressDialog->setWindowModality(Qt::ApplicationModal);
    progressDialog->setWindowFlag(Qt::FramelessWindowHint);
    progressDialog->setCancelButton(nullptr);
    progressDialog->setMinimumDuration(0);
    progressDialog->setValue(0);
    progressDialog->show();

    int index = item->listWidget()->currentRow();
    long statusCode = apiClient.Download(index);

    QMessageBox messageBox(this);
    messageBox.setWindowModality(Qt::ApplicationModal);
    messageBox.setWindowFlag(Qt::FramelessWindowHint);

    QMetaObject::invokeMethod(progressDialog, "reset");
    delete progressDialog;

    if (statusCode == 200)
    {
        messageBox.setText("File downloaded succefully! Please check your stego_media folder.");
        messageBox.exec();
        return;
    }
    else
    {
        messageBox.setText("Failed to download file. Please check that your internet connection and try again.");
        messageBox.exec();
        return;
    }
}
