#include <QTest>
#include "../stego.h"

class UnitTests: public QObject
{
    Q_OBJECT
private:
    QString testEmbedImage3KB = "TestFiles/image_3KB.png";
    QString testEmbedImage16KB = "TestFiles/image_16KB.png";
    QString testEmbedImage207KB = "TestFiles/image_207KB.jpg";
    QString testEmbedImage773KB = "TestFiles/image_773KB.jpg";

    QString testEmbedPdf1KB = "TestFiles/pdf_1KB.pdf";
    QString testEmbedPdf19KB = "TestFiles/pdf_19KB.pdf";
    QString testEmbedPdf26KB = "TestFiles/pdf_26KB.pdf";
    QString testEmbedPdf664KB = "TestFiles/pdf_664KB.pdf";
    QString testEmbedPdf5MB = "TestFiles/pdf_5MB.pdf";

    QString testEmbedMp3_5KB = "TestFiles/mp3_5KB.mp3";
    QString testEmbedMp3_27KB = "TestFiles/mp3_27KB.MP3";
    QString testEmbedMp3_700KB = "TestFiles/mp3_700KB.mp3";
    QString testEmbedMp3_5MB = "TestFiles/mp3_5MB.mp3";


    QString testEmbedZip_5KB = "TestFiles/zip_5KB.zip";
    QString testEmbedZip_626KB = "TestFiles/zip_626KB.zip";
    QString testEmbedZip_5MB = "TestFiles/zip_5MB.zip";
    QString testEmbedZip_50MB = "TestFiles/zip_50MB.zip";

    QString smallImage = "TestMedia/Lenna.png";
    QString largeImage = "TestMedia/largeImage.png";
    QString testVideo = "TestMedia/sample_640x360.mkv";
    QString testVideoWithAudio = "TestMedia/sample_960x400_ocean_with_audio.mkv";

    QString LSB = "LSB";
    QString PVD = "PVD";

    QString noEdgeDetection = "None";
    QString sobelEdgeDetection = "Sobel";
    QString cannyEdgeDetection = "Canny";

    QString emptyPassword = "";

private slots:
    void embedAndExtractFileInMediaTest_data()
    {
        QTest::addColumn<QString>("file");
        QTest::addColumn<QString>("media");
        QTest::addColumn<QString>("stegoAlgo");
        QTest::addColumn<QString>("edgeDetection");
        QTest::addColumn<bool>("encryption");
        QTest::addColumn<QString>("password");

        // 1
        QTest::newRow("image16KB-smallImage-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedImage16KB << smallImage << LSB << noEdgeDetection << false << emptyPassword;

        // 2
        QTest::newRow("image16KB-smallImage-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedImage16KB << smallImage << LSB << sobelEdgeDetection << false << emptyPassword;

        // 3
        QTest::newRow("image3KB-smallImage-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedImage3KB << smallImage << LSB << cannyEdgeDetection << false << emptyPassword;

        // 4
        QTest::newRow("pdf26KB-smallImage-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedPdf26KB << smallImage << LSB << noEdgeDetection << false << emptyPassword;

        // 5
        QTest::newRow("pdf26KB-smallImage-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedPdf26KB << smallImage << LSB << sobelEdgeDetection << false << emptyPassword;

        // 6
        QTest::newRow("pdf1KB-smallImage-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedPdf1KB << smallImage << LSB << cannyEdgeDetection << false << emptyPassword;

        // 7
        QTest::newRow("mp3_27KB-smallImage-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedMp3_27KB << smallImage << LSB << noEdgeDetection << false << emptyPassword;

        // 8
        QTest::newRow("mp3_27KB-smallImage-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedMp3_27KB << smallImage << LSB << sobelEdgeDetection << false << emptyPassword;

        // 9
        QTest::newRow("mp3_5KB-smallImage-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedMp3_5KB << smallImage << LSB << cannyEdgeDetection << false << emptyPassword;

        // 10
        QTest::newRow("zip_5KB-smallImage-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedZip_5KB << smallImage << LSB << noEdgeDetection << false << emptyPassword;

        // 11
        QTest::newRow("zip_5KB-smallImage-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedZip_5KB << smallImage << LSB << sobelEdgeDetection << false << emptyPassword;

        // 12
        QTest::newRow("zip_5KB-smallImage-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedZip_5KB << smallImage << LSB << cannyEdgeDetection << false << emptyPassword;

        // 13
        QTest::newRow("image16KB-smallImage-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedImage16KB << smallImage << PVD << noEdgeDetection << false << emptyPassword;

        // 14
        QTest::newRow("image16KB-smallImage-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedImage16KB << smallImage << PVD << sobelEdgeDetection << false << emptyPassword;

        // 15
        QTest::newRow("image3KB-smallImage-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedImage3KB << smallImage << PVD << cannyEdgeDetection << false << emptyPassword;

        // 16
        QTest::newRow("pdf26KB-smallImage-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedPdf26KB << smallImage << PVD << noEdgeDetection << false << emptyPassword;

        // 17
        QTest::newRow("pdf26KB-smallImage-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedPdf26KB << smallImage << PVD << sobelEdgeDetection << false << emptyPassword;

        // 18
        QTest::newRow("pdf1KB-smallImage-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedPdf1KB << smallImage << PVD << cannyEdgeDetection << false << emptyPassword;

        // 19
        QTest::newRow("mp3_27KB-smallImage-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedMp3_27KB << smallImage << PVD << noEdgeDetection << false << emptyPassword;

        // 20
        QTest::newRow("mp3_27KB-smallImage-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedMp3_27KB << smallImage << PVD << sobelEdgeDetection << false << emptyPassword;

        // 21
        QTest::newRow("mp3_5KB-smallImage-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedMp3_5KB << smallImage << PVD << cannyEdgeDetection << false << emptyPassword;

        // 22
        QTest::newRow("zip_5KB-smallImage-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedZip_5KB << smallImage << PVD << noEdgeDetection << false << emptyPassword;

        // 23
        QTest::newRow("zip_5KB-smallImage-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedZip_5KB << smallImage << PVD << sobelEdgeDetection << false << emptyPassword;

        // 24
        QTest::newRow("zip_5KB-smallImage-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedZip_5KB << smallImage << PVD << cannyEdgeDetection << false << emptyPassword;

        // 25
        QTest::newRow("image773KB-largeImage-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << largeImage << LSB << noEdgeDetection << false << emptyPassword;

        // 26
        QTest::newRow("image207KB-largeImage-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedImage207KB << largeImage << LSB << sobelEdgeDetection << false << emptyPassword;

        // 27
        QTest::newRow("image16KB-largeImage-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedImage16KB << largeImage << LSB << cannyEdgeDetection << false << emptyPassword;

        // 28
        QTest::newRow("pdf664KB-largeImage-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedPdf664KB << largeImage << LSB << noEdgeDetection << false << emptyPassword;

        // 29
        QTest::newRow("pdf664KB-largeImage-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedPdf664KB << largeImage << LSB << sobelEdgeDetection << false << emptyPassword;

        // 30
        QTest::newRow("pdf19KB-largeImage-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedPdf19KB << largeImage << LSB << cannyEdgeDetection << false << emptyPassword;

        // 31
        QTest::newRow("mp3_700KB-largeImage-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedMp3_700KB << largeImage << LSB << noEdgeDetection << false << emptyPassword;

        // 32
        QTest::newRow("mp3_700KB-largeImage-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedMp3_700KB << largeImage << LSB << sobelEdgeDetection << false << emptyPassword;

        // 33
        QTest::newRow("mp3_5KB-largeImage-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedMp3_5KB << largeImage << LSB << cannyEdgeDetection << false << emptyPassword;

        // 34
        QTest::newRow("zip_626KB-largeImage-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedZip_626KB << largeImage << LSB << noEdgeDetection << false << emptyPassword;

        // 35
        QTest::newRow("zip_626KB-largeImage-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedZip_626KB << largeImage << LSB << sobelEdgeDetection << false << emptyPassword;

        // 36
        QTest::newRow("zip_5KB-largeImage-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedZip_5KB << largeImage << LSB << cannyEdgeDetection << false << emptyPassword;

        // 37
        QTest::newRow("image773KB-largeImage-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << largeImage << PVD << noEdgeDetection << false << emptyPassword;

        // 38
        QTest::newRow("image207KB-largeImage-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedImage207KB << largeImage << PVD << sobelEdgeDetection << false << emptyPassword;

        // 39
        QTest::newRow("image3KB-largeImage-PVD-cannyEdgeDetection-NoEncryption") <<
            testEmbedImage3KB << largeImage << PVD << cannyEdgeDetection << false << emptyPassword;

        // 40
        QTest::newRow("pdf664KB-largeImage-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedPdf664KB << largeImage << PVD << noEdgeDetection << false << emptyPassword;

        // 41
        QTest::newRow("pdf26KB-largeImage-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedPdf26KB << largeImage << PVD << sobelEdgeDetection << false << emptyPassword;

        // 42
        QTest::newRow("pdf1KB-largeImage-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedPdf1KB << largeImage << PVD << cannyEdgeDetection << false << emptyPassword;

        // 43
        QTest::newRow("mp3_700KB-largeImage-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedMp3_700KB << largeImage << PVD << noEdgeDetection << false << emptyPassword;

        // 44
        QTest::newRow("mp3_27KB-largeImage-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedMp3_27KB << largeImage << PVD << sobelEdgeDetection << false << emptyPassword;

        // 45
        QTest::newRow("mp3_5KB-largeImage-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedMp3_5KB << largeImage << PVD << cannyEdgeDetection << false << emptyPassword;

        // 46
        QTest::newRow("zip626KB-largeImage-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedZip_626KB << largeImage << PVD << noEdgeDetection << false << emptyPassword;

        // 47
        QTest::newRow("zip5KB-largeImage-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedZip_5KB << largeImage << PVD << sobelEdgeDetection << false << emptyPassword;

        // 48
        QTest::newRow("zip5KB-largeImage-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedZip_5KB << largeImage << PVD << cannyEdgeDetection << false << emptyPassword;

        // 49
        QTest::newRow("image773KB-testVideo-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << testVideo << LSB << noEdgeDetection << false << emptyPassword;

        // 50
        QTest::newRow("image207KB-testVideo-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedImage207KB << testVideo << LSB << sobelEdgeDetection << false << emptyPassword;

        // 51
        QTest::newRow("image207KB-testVideo-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedImage207KB << testVideo << LSB << cannyEdgeDetection << false << emptyPassword;

        // 52
        QTest::newRow("pdf5MB-testVideo-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedPdf5MB << testVideo << LSB << noEdgeDetection << false << emptyPassword;

        // 53
        QTest::newRow("pdf5MB-testVideo-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedPdf5MB << testVideo << LSB << sobelEdgeDetection << false << emptyPassword;

        // 54
        QTest::newRow("pdf26KB-testVideo-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedPdf26KB << testVideo << LSB << cannyEdgeDetection << false << emptyPassword;

        // 55
        QTest::newRow("mp3_5MB-testVideo-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedMp3_5MB << testVideo << LSB << noEdgeDetection << false << emptyPassword;

        // 56
        QTest::newRow("mp3_5MB-testVideo-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedMp3_5MB << testVideo << LSB << sobelEdgeDetection << false << emptyPassword;

        // 57
        QTest::newRow("mp3_27KB-testVideo-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedMp3_27KB << testVideo << LSB << cannyEdgeDetection << false << emptyPassword;

        // 58
        QTest::newRow("zip5MB-testVideo-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedZip_5MB << testVideo << LSB << noEdgeDetection << false << emptyPassword;

        // 59
        QTest::newRow("zip5MB-testVideo-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedZip_5MB << testVideo << LSB << sobelEdgeDetection << false << emptyPassword;

        // 60
        QTest::newRow("zip5KB-testVideo-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedZip_5KB << testVideo << LSB << cannyEdgeDetection << false << emptyPassword;

        // 61
        QTest::newRow("image773KB-testVideo-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << testVideo << PVD << noEdgeDetection << false << emptyPassword;

        // 62
        QTest::newRow("image773KB-testVideo-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << testVideo << PVD << sobelEdgeDetection << false << emptyPassword;

        // 63
        QTest::newRow("image207KB-testVideo-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedImage207KB << testVideo << PVD << cannyEdgeDetection << false << emptyPassword;

        // 64
        QTest::newRow("pdf5MB-testVideo-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedPdf5MB << testVideo << PVD << noEdgeDetection << false << emptyPassword;

        // 65
        QTest::newRow("pdf5MB-testVideo-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedPdf5MB << testVideo << PVD << sobelEdgeDetection << false << emptyPassword;

        // 66
        QTest::newRow("pdf26KB-testVideo-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedPdf26KB << testVideo << PVD << cannyEdgeDetection << false << emptyPassword;

        // 67
        QTest::newRow("mp3_5MB-testVideo-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedMp3_5MB << testVideo << PVD << noEdgeDetection << false << emptyPassword;

        // 68
        QTest::newRow("mp3_5MB-testVideo-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedMp3_5MB << testVideo << PVD << sobelEdgeDetection << false << emptyPassword;

        // 69
        QTest::newRow("mp3_27KB-testVideo-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedMp3_27KB << testVideo << PVD << cannyEdgeDetection << false << emptyPassword;

        // 70
        QTest::newRow("zip5MB-testVideo-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedZip_5MB << testVideo << PVD << noEdgeDetection << false << emptyPassword;

        // 71
        QTest::newRow("zip5MB-testVideo-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedZip_5MB << testVideo << PVD << sobelEdgeDetection << false << emptyPassword;

        // 72
        QTest::newRow("zip5KB-testVideo-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedZip_5KB << testVideo << PVD << cannyEdgeDetection << false << emptyPassword;

        // 73
        QTest::newRow("image773KB-testVideoWithAudio-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << testVideoWithAudio << LSB << noEdgeDetection << false << emptyPassword;

        // 74
        QTest::newRow("image773KB-testVideoWithAudio-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << testVideoWithAudio << LSB << sobelEdgeDetection << false << emptyPassword;

        // 75
        QTest::newRow("image207KB-testVideoWithAudio-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedImage207KB << testVideoWithAudio << LSB << cannyEdgeDetection << false << emptyPassword;

        // 76
        QTest::newRow("pdf5MB-testVideoWithAudio-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedPdf5MB << testVideoWithAudio << LSB << noEdgeDetection << false << emptyPassword;

        // 77
        QTest::newRow("pdf5MB-testVideoWithAudio-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedPdf5MB << testVideoWithAudio << LSB << sobelEdgeDetection << false << emptyPassword;

        // 78
        QTest::newRow("pdf664KB-testVideoWithAudio-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedPdf664KB << testVideoWithAudio << LSB << cannyEdgeDetection << false << emptyPassword;

        // 79
        QTest::newRow("mp3_5MB-testVideoWithAudio-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedMp3_5MB << testVideoWithAudio << LSB << noEdgeDetection << false << emptyPassword;

        // 80
        QTest::newRow("mp3_5MB-testVideoWithAudio-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedMp3_5MB << testVideoWithAudio << LSB << sobelEdgeDetection << false << emptyPassword;

        // 81
        QTest::newRow("mp3_700KB-testVideoWithAudio-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedMp3_700KB << testVideoWithAudio << LSB << cannyEdgeDetection << false << emptyPassword;

        // 82
        QTest::newRow("zip5MB-testVideoWithAudio-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedZip_5MB << testVideoWithAudio << LSB << noEdgeDetection << false << emptyPassword;

        // 83
        QTest::newRow("zip5MB-testVideoWithAudio-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedZip_5MB << testVideoWithAudio << LSB << sobelEdgeDetection << false << emptyPassword;

        // 84
        QTest::newRow("zip626KB-testVideoWithAudio-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedZip_626KB << testVideoWithAudio << LSB << cannyEdgeDetection << false << emptyPassword;

        // 85
        QTest::newRow("image773KB-testVideoWithAudio-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << testVideoWithAudio << PVD << noEdgeDetection << false << emptyPassword;

        // 86
        QTest::newRow("image773KB-testVideoWithAudio-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << testVideoWithAudio << PVD << sobelEdgeDetection << false << emptyPassword;

        // 87
        QTest::newRow("image773KB-testVideoWithAudio-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << testVideoWithAudio << PVD << cannyEdgeDetection << false << emptyPassword;

        // 88
        QTest::newRow("pdf5MB-testVideoWithAudio-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedPdf5MB << testVideoWithAudio << PVD << noEdgeDetection << false << emptyPassword;

        // 89
        QTest::newRow("pdf5MB-testVideoWithAudio-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedPdf5MB << testVideoWithAudio << PVD << sobelEdgeDetection << false << emptyPassword;

        // 90
        QTest::newRow("pdf664KB-testVideoWithAudio-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedPdf664KB << testVideoWithAudio << PVD << cannyEdgeDetection << false << emptyPassword;

        // 91
        QTest::newRow("mp3_5MB-testVideoWithAudio-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedMp3_5MB << testVideoWithAudio << PVD << noEdgeDetection << false << emptyPassword;

        // 92
        QTest::newRow("mp3_5MB-testVideoWithAudio-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedMp3_5MB << testVideoWithAudio << PVD << sobelEdgeDetection << false << emptyPassword;

        // 93
        QTest::newRow("mp3_700KB-testVideoWithAudio-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedMp3_700KB << testVideoWithAudio << PVD << cannyEdgeDetection << false << emptyPassword;

        // 94
        QTest::newRow("zip5MB-testVideoWithAudio-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedZip_5MB << testVideoWithAudio << PVD << noEdgeDetection << false << emptyPassword;

        // 95
        QTest::newRow("zip5MB-testVideoWithAudio-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedZip_5MB << testVideoWithAudio << PVD << sobelEdgeDetection << false << emptyPassword;

        // 96
        QTest::newRow("zip626KB-testVideoWithAudio-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedZip_626KB << testVideoWithAudio << PVD << cannyEdgeDetection << false << emptyPassword;
    }

    void embedAndExtractFileInMediaTest()
    {
        QFETCH(QString, file);
        QFETCH(QString, media);
        QFETCH(QString, stegoAlgo);
        QFETCH(QString, edgeDetection);
        QFETCH(bool, encryption);
        QFETCH(QString, password);

        Stego encodeStego = Stego(file.toStdString(),
                            media.toStdString(),
                            stegoAlgo.toStdString(),
                            edgeDetection.toStdString(),
                            encryption,
                            password.toStdString());

        StegoStatus status;
        if (encryption)
        {
            status = encodeStego.EncryptFile();
            QCOMPARE(status, StegoStatus::SUCCESS);
        }

        if (media.endsWith(".png"))
        {
            status = encodeStego.EncodeImage();
        }
        else if (media.endsWith(".mkv"))
        {
            status = encodeStego.EncodeVideo();
        }

        QCOMPARE(status, StegoStatus::SUCCESS);

        std::string mediaName = std::filesystem::path(media.toStdString()).filename().string();
        Stego decodeStego = Stego("stego_media/" + mediaName, encryption, password.toStdString());

        if (media.endsWith(".png"))
        {
            status = decodeStego.DecodeImage();
        }
        else if (media.endsWith(".mkv"))
        {
            status = decodeStego.DecodeVideo();
        }

        QCOMPARE(status, StegoStatus::SUCCESS);

        if (encryption)
        {
            status = decodeStego.DecryptFile();
            QCOMPARE(status, StegoStatus::SUCCESS);
        }

        std::ostringstream command;
        std::string fileName = std::filesystem::path(file.toStdString()).filename().string();
        command << "cmp " << file.toStdString() << " decoded_files/" << fileName;
        std::system(command.str().c_str());
        int result = system(command.str().c_str());

        QCOMPARE(result, 0);
    }

    void embedTooLargeFile_data()
    {
        QTest::addColumn<QString>("file");
        QTest::addColumn<QString>("media");
        QTest::addColumn<QString>("stegoAlgo");
        QTest::addColumn<QString>("edgeDetection");
        QTest::addColumn<bool>("encryption");
        QTest::addColumn<QString>("password");

        // 97
        QTest::newRow("image773KB-smallImage-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << smallImage << LSB << noEdgeDetection << false << emptyPassword;

        // 98
        QTest::newRow("image773KB-smallImage-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << smallImage << LSB << sobelEdgeDetection << false << emptyPassword;

        // 99
        QTest::newRow("image773KB-smallImage-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedImage773KB << smallImage << LSB << cannyEdgeDetection << false << emptyPassword;

        // 100
        QTest::newRow("zip_50MB-smallImage-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedZip_50MB << smallImage << PVD << noEdgeDetection << false << emptyPassword;

        // 101
        QTest::newRow("mp3_5MB-smallImage-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedMp3_5MB << smallImage << PVD << sobelEdgeDetection << false << emptyPassword;

        // 102
        QTest::newRow("mp3_5MB-smallImage-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedMp3_5MB << smallImage << PVD << cannyEdgeDetection << false << emptyPassword;

        // 103
        QTest::newRow("zip50MB-testVideo-LSB-NoEdgeDetection-NoEncryption") <<
            testEmbedZip_50MB << testVideo << LSB << noEdgeDetection << false << emptyPassword;

        // 104
        QTest::newRow("zip50MB-testVideo-LSB-SobelEdgeDetection-NoEncryption") <<
            testEmbedZip_50MB << testVideo << LSB << sobelEdgeDetection << false << emptyPassword;

        // 105
        QTest::newRow("zip50MB-testVideo-LSB-CannyEdgeDetection-NoEncryption") <<
            testEmbedZip_50MB << testVideo << LSB << cannyEdgeDetection << false << emptyPassword;

        // 106
        QTest::newRow("zip50MB-testVideo-PVD-NoEdgeDetection-NoEncryption") <<
            testEmbedZip_50MB << testVideo << PVD << noEdgeDetection << false << emptyPassword;

        // 107
        QTest::newRow("zip50MB-testVideo-PVD-SobelEdgeDetection-NoEncryption") <<
            testEmbedZip_50MB << testVideo << PVD << sobelEdgeDetection << false << emptyPassword;

        // 108
        QTest::newRow("zip50MB-testVideo-PVD-CannyEdgeDetection-NoEncryption") <<
            testEmbedZip_50MB << testVideo << PVD << cannyEdgeDetection << false << emptyPassword;
    }


    void embedTooLargeFile()
    {
        QFETCH(QString, file);
        QFETCH(QString, media);
        QFETCH(QString, stegoAlgo);
        QFETCH(QString, edgeDetection);
        QFETCH(bool, encryption);
        QFETCH(QString, password);

        Stego encodeStego = Stego(file.toStdString(),
                                  media.toStdString(),
                                  stegoAlgo.toStdString(),
                                  edgeDetection.toStdString(),
                                  encryption,
                                  password.toStdString());

        StegoStatus status;
        if (encryption)
        {
            status = encodeStego.EncryptFile();
            QCOMPARE(status, StegoStatus::SUCCESS);
        }

        if (media.endsWith(".png"))
        {
            status = encodeStego.EncodeImage();
        }
        else if (media.endsWith(".mkv"))
        {
            status = encodeStego.EncodeVideo();
        }

        QCOMPARE(status, StegoStatus::FILE_TOO_LARGE);
    }
};

QTEST_MAIN(UnitTests)
#include "UnitTests.moc"
