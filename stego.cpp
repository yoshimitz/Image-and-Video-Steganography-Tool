#include "stego.h"
#include "edgedetection.h"
#include <filesystem>
#include <fstream>
#include <QDebug>
#include <cstdio>

using namespace cv;

const size_t NUM_HEADER_PIXELS = 10;
const size_t LSB_BITS_PER_PIXEL = 3;
const size_t BITS_PER_BYTE = 8;
const size_t MAX_PVD_GREEN_EMBEDDING = 3;
const size_t MAX_PVD_RED_EMBEDDING = 5;
const size_t MAX_PVD_BLUE_EMBEDDING = 7;


std::string tempEncryptFilePath = "temp.encrypt";

uchar setBit(uchar number, int position)
{
    return (number | (1 << (position)));
}

uchar clearBit(uchar number, int position)
{
    return (number & (~(1 << (position))));
}


Stego::Stego(std::string filePath, std::string mediaPath, std::string algo, std::string edgeDetection,
             bool bEncrypt, std::string password)
    : filePath(filePath)
    , mediaPath(mediaPath)
    , fileNameLength(0)
    , fileLength(0)
    , bEncrypt(bEncrypt)
    , password(password)
    , currentRow(0)
    , currentColumn(0)
{
    if (algo == "LSB")
    {
        this->algo = StegoAlgo::LSB;
    }
    else if (algo == "PVD")
    {
        this->algo = StegoAlgo::PVD;
    }

    if (edgeDetection == "None")
    {
        this->edgeDetectionType = EdgeDetectionType::None;
    }
    else if (edgeDetection == "Canny")
    {
        this->edgeDetectionType = EdgeDetectionType::Canny;
    }
    else if (edgeDetection == "Sobel")
    {
        this->edgeDetectionType = EdgeDetectionType::Sobel;
    }

    this->fileName = std::filesystem::path(this->filePath).filename().string();
}

Stego::Stego(std::string mediaPath, bool bEncrypt, std::string password)
    : mediaPath(mediaPath)
    , fileNameLength(0)
    , fileLength(0)
    , bEncrypt(bEncrypt)
    , password(password)
    , fileName("")
    , currentRow(0)
    , currentColumn(0)
{
}


Stego::~Stego() {
    if (std::filesystem::exists(tempEncryptFilePath))
    {
        std::remove(tempEncryptFilePath.c_str());
    }
}


StegoStatus Stego::EncodeImage()
{
    Mat image = imread(this->mediaPath, IMREAD_COLOR);

    if (this->edgeDetectionType != EdgeDetectionType::None)
    {
        this->edgeDetector = EdgeDetection();
        this->edgeDetector.DetectEdges(image, this->edgeDetectionType);
    }

    if (this->isFileTooLarge(image))
    {
        return StegoStatus::FILE_TOO_LARGE;
    }

    encodeHeader(image);

    if (this->algo == StegoAlgo::LSB)
    {
        return encodeLsb(image);
    }
    else if (this->algo == StegoAlgo::PVD)
    {
        return encodePvd(image);
    }

    return StegoStatus::SUCCESS;
}

StegoStatus Stego::EncryptFile()
{
    std::ifstream file(this->filePath, std::ios_base::binary);
    std::ofstream encryptedFile(tempEncryptFilePath, std::ios::binary);
    if (!file.is_open() || !encryptedFile.is_open()) {
        return StegoStatus::FILE_OPEN_FAILED;
    }

    // Create FileSink and DefaultEncryptorWithMAC objects
    CryptoPP::DefaultEncryptorWithMAC encryptor((CryptoPP::byte*)password.data(), password.size(), new CryptoPP::FileSink(encryptedFile));

    // Create Redirector and FileSource objects
    CryptoPP::FileSource fileSource(file, true, new CryptoPP::Redirector(encryptor));

    this->filePath = tempEncryptFilePath;

    return StegoStatus::SUCCESS;
}

StegoStatus Stego::DecodeImage()
{
    Mat image = imread(this->mediaPath, IMREAD_COLOR);
    size_t rowIdx = 0;
    size_t colIdx = 0;

    StegoStatus status = decodeHeader(image);
    if (status != StegoStatus::SUCCESS)
    {
        return status;
    }

    if (this->edgeDetectionType != EdgeDetectionType::None)
    {
        this->edgeDetector = EdgeDetection();
        this->edgeDetector.DetectEdges(image, this->edgeDetectionType);
    }

    if (this->algo == StegoAlgo::LSB)
    {
        return decodeLsb(image);
    }
    else if (this->algo == StegoAlgo::PVD)
    {
        return decodePvd(image);
    }

    return status;
}

StegoStatus Stego::EncodeVideo()
{
    VideoCapture video(this->mediaPath);
    if (!video.isOpened())
    {
        qDebug()  << "Could not open video " << this->mediaPath;
        return StegoStatus::VIDEO_OPEN_FAILED;
    }

    Mat frame;
    video >> frame;
    encodeHeader(frame);

    if (this->edgeDetectionType != EdgeDetectionType::None)
    {
        this->edgeDetector = EdgeDetection();
        this->edgeDetector.DetectEdges(frame, this->edgeDetectionType);
    }

    // Estimate size based on initial frame.
    if (this->isFileTooLarge(frame, video.get(CAP_PROP_FRAME_COUNT)))
    {
        return StegoStatus::FILE_TOO_LARGE;
    }

    StegoStatus status;
    std::ifstream file(this->filePath, std::ios_base::binary);

    // Encode file name and file into first frame
    if (this->algo == StegoAlgo::LSB)
    {
        status = encodeLsbFileName(frame);
        if (status != StegoStatus::SUCCESS)
        {
            return status;
        }

        status = encodeLsbFile(frame, file);
        if (status != StegoStatus::SUCCESS)
        {
            return status;
        }
    }
    else if (this->algo == StegoAlgo::PVD)
    {
        // status = encodePvdFileName(frame);
        if (status != StegoStatus::SUCCESS)
        {
            return status;
        }
    }

    // Create stego media directory, if it does not exist
    std::filesystem::create_directory("stego_media");

    // Create video writer for writing encoded frames to.
    std::string mediaName = std::filesystem::path(this->mediaPath).filename().string();
    std::string stegoMediaPath = "stego_media/" + mediaName;
    std::string tempStegoMediaPath = "stego_media/temp_" + mediaName;
    Size videoSize = Size((int) video.get(CAP_PROP_FRAME_WIDTH), (int) video.get(CAP_PROP_FRAME_HEIGHT));
    double videoFPS = video.get(CAP_PROP_FPS);

    std::string pipeline = "appsrc ! videoconvert ! x264enc quantizer=0 ! video/x-h264,profile=high ! mp4mux ! filesink location='" + tempStegoMediaPath + "'";
    cv::VideoWriter outputVideo(pipeline, cv::CAP_GSTREAMER, 0, videoFPS, videoSize, true);

    //outputVideo.open(tempStegoMediaPath, VideoWriter::fourcc('X', '2', '6', '4'),
    //                videoFPS, videoSize, true);

    if (!outputVideo.isOpened())
    {
        qDebug()  << "Could not open video " << this->mediaPath;
        return StegoStatus::VIDEO_OPEN_FAILED;
    }

    outputVideo << frame;
    video >> frame;
    size_t numFrames = 1;

    while (!frame.empty())
    {
        if (this->edgeDetectionType != EdgeDetectionType::None)
        {
            this->edgeDetector.DetectEdges(frame, this->edgeDetectionType);
        }

        if (this->algo == StegoAlgo::LSB)
        {
            status = encodeLsbFile(frame, file);
            if (status != StegoStatus::SUCCESS)
            {
                return status;
            }
        }
        else if (this->algo == StegoAlgo::PVD)
        {
            return encodePvd(frame);
        }

        outputVideo << frame;
        video >> frame;

        numFrames++;
        if (file.eof())
        {
            break;
        }
    }

    // Write the rest of the unencoded frames to new file.
    while (!frame.empty())
    {
        outputVideo << frame;
        video >> frame;
        numFrames++;
    }

    video.release();
    outputVideo.release();
    file.close();

    //std::string command = "ffmpeg -i " + tempStegoMediaPath + " -i " + this->mediaPath + " -map 0:v -map 1:a -c:v copy -c:a copy " + stegoMediaPath;
    //std::system(command.c_str());

    return StegoStatus::SUCCESS;
}

StegoStatus Stego::DecodeVideo()
{

    return StegoStatus::SUCCESS;
}

StegoStatus Stego::DecryptFile()
{
    std::ifstream file(tempEncryptFilePath, std::ios_base::binary);
    std::ofstream decryptedFile("decoded_files/" + this->fileName, std::ios::binary);
    if (!file.is_open() || !decryptedFile.is_open()) {
        return StegoStatus::FILE_OPEN_FAILED;
    }

    try {
        // Create FileSink and DefaultDecryptor objects
        CryptoPP::DefaultDecryptorWithMAC decryptor((CryptoPP::byte*)password.data(), password.size(),
                                             new CryptoPP::FileSink(decryptedFile));

        // Create Redirector and FileSource objects
        CryptoPP::FileSource fileSource(file, true, new CryptoPP::Redirector(decryptor));
    }
    catch (const CryptoPP::Exception& e) {
        qDebug() << "Crypto++ exception: " << e.what();
        std::remove(("decoded_files/" + this->fileName).c_str());

        return StegoStatus::DECRYPTION_FAILED;
    }

    return StegoStatus::SUCCESS;
}

/**
 * Uses the LSB stego method to encode data in a image/video frame.
 * @brief Stego::encodeLsb
 */
StegoStatus Stego::encodeLsb(Mat image)
{
    StegoStatus status = encodeLsbFileName(image);
    if (status != StegoStatus::SUCCESS)
    {
        return status;
    }

    std::ifstream file(this->filePath, std::ios_base::binary);

    status = encodeLsbFile(image, file);
    if (status != StegoStatus::SUCCESS)
    {
        return status;
    }

    std::filesystem::create_directory("stego_media");
    std::string mediaName = std::filesystem::path(this->mediaPath).filename().string();
    imwrite(std::filesystem::path("stego_media/" + mediaName), image);
    file.close();

    return status;
}

StegoStatus Stego::encodePvd(Mat image)
{
    StegoStatus status = encodeLsbFileName(image);
    if (status != StegoStatus::SUCCESS)
    {
        return status;
    }

    return encodePvdData();
}

void Stego::encodeHeader(Mat image)
{
    int channels = image.channels();
    int nRows = image.rows;
    int nCols = image.cols * channels;
    if (image.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    uchar* row = image.ptr<uchar>(0);

    // Embed Algorithm in lsb of blue segment of first pixel
    if (this->algo == StegoAlgo::LSB)
    {
        // 0
        row[0] = clearBit(row[0], 0);
    }
    else
    {
        // 1
        row[0] = setBit(row[0], 0);
    }

    // Embed Edge Detection in least 2 significant bits of green segment of first pixel
    if (this->edgeDetectionType == EdgeDetectionType::None)
    {
        // 00
        row[1] = clearBit(row[1], 0);
        row[1] = clearBit(row[1], 1);
    }
    else if (this->edgeDetectionType == EdgeDetectionType::Canny)
    {
        // 01
        row[1] = setBit(row[1], 0);
        row[1] = clearBit(row[1], 1);
    }
    else if (this->edgeDetectionType == EdgeDetectionType::Sobel)
    {
        // 11
        row[1] = setBit(row[1], 0);
        row[1] = setBit(row[1], 1);
    }

    // Encryption status in least significant bit of red segment of first pixel
    if (this->bEncrypt)
    {
        // 1
        row[2] = setBit(row[2], 0);
    }
    else
    {
        // 0
        row[2] = clearBit(row[2], 0);
    }

    // Embed File Name Length in Pixels 2-4
    uint16_t fileNameLength =  this->fileName.length();
    std::bitset<18> fileNameLengthBits(fileNameLength);

    size_t bitPos = 0;
    this->currentColumn = 3;
    for (; currentRow < nRows; currentRow++)
    {
        row = image.ptr<uchar>(currentRow);
        for (; currentColumn < nCols; currentColumn++)
        {
            if (fileNameLengthBits[bitPos])
            {
                row[currentColumn] = setBit(row[currentColumn], 0);
            }
            else
            {
                row[currentColumn] = clearBit(row[currentColumn], 0);
            }

            if (fileNameLengthBits[bitPos + 1])
            {
                row[currentColumn] = setBit(row[currentColumn], 1);
            }
            else
            {
                row[currentColumn] = clearBit(row[currentColumn], 1);
            }

            bitPos += 2;
            if (bitPos >= fileNameLengthBits.size())
            {
                break;
            }
        }

        if (bitPos >= fileNameLengthBits.size())
        {
            break;
        }

        currentColumn = 0;
    }

    // Embed File Length in Pixels 5-10
    uint32_t fileLength = std::filesystem::file_size(this->filePath);
    std::bitset<36> fileLengthBits(fileLength);
    bitPos = 0;
    for (; currentRow < nRows; currentRow++)
    {
        row = image.ptr<uchar>(currentRow);
        for (; currentColumn < nCols; currentColumn++)
        {
            if (fileLengthBits[bitPos])
            {
                row[currentColumn] = setBit(row[currentColumn], 0);
            }
            else
            {
                row[currentColumn] = clearBit(row[currentColumn], 0);
            }

            if (fileLengthBits[bitPos + 1])
            {
                row[currentColumn] = setBit(row[currentColumn], 1);
            }
            else
            {
                row[currentColumn] = clearBit(row[currentColumn], 1);
            }

            bitPos += 2;
            if (bitPos >= fileLengthBits.size())
            {
                break;
            }
        }

        if (bitPos >= fileLengthBits.size())
        {
            break;
        }

        currentColumn = 0;
    }
}

StegoStatus Stego::encodeLsbFileName(cv::Mat image)
{
    int channels = image.channels();
    int nRows = image.rows;
    int nCols = image.cols * channels;
    if (image.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    Mat edges;
    uchar* edgeRow;
    size_t edgeCol = 0;
    if (edgeDetectionType != EdgeDetectionType::None)
    {
        edges = edgeDetector.getMagnitudes();
        edgeRow = edges.ptr<uchar>(currentRow);
        edgeCol = currentColumn / 3;
    }

    uchar* imageRow = image.ptr<uchar>(currentRow);

    std::bitset<8> dataByte;
    for (size_t i = 0; i < this->fileName.size(); i++)
    {
        dataByte = std::bitset<8>(this->fileName[i]);
        size_t j = 0;
        while (j < dataByte.size())
        {
            if (edgeDetectionType != EdgeDetectionType::None)
            {
                // NOLINTBEGIN
                if (edgeRow[edgeCol] == 0)
                // NOLINTEND
                {
                    currentColumn++;
                    edgeCol = currentColumn / 3;
                    continue;
                }
            }

            if (dataByte[j])
            {
                imageRow[currentColumn] = setBit(imageRow[currentColumn], 0);
            }
            else
            {
                imageRow[currentColumn] = clearBit(imageRow[currentColumn], 0);
            }

            j++;
            currentColumn++;
            edgeCol = currentColumn / 3;

            if (currentColumn >= nCols)
            {
                currentColumn = 0;
                edgeCol = 0;
                currentRow++;
                if (currentRow >= nRows)
                {
                    // Shouldn't happen. For debugging.
                    return StegoStatus::OUT_OF_ROOM;
                }
                else
                {
                    imageRow = image.ptr<uchar>(currentRow);

                    if (edgeDetectionType != EdgeDetectionType::None)
                    {
                        edgeRow = edges.ptr<uchar>(currentRow);
                    }
                }
            }
        }
    }

    return StegoStatus::SUCCESS;
}

StegoStatus Stego::encodeLsbFile(Mat image, std::ifstream& file)
{
    int channels = image.channels();
    int nRows = image.rows;
    int nCols = image.cols * channels;
    if (image.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    Mat edges;
    uchar* edgeRow;
    size_t edgeCol = 0;
    if (edgeDetectionType != EdgeDetectionType::None)
    {
        edges = edgeDetector.getMagnitudes();
        edgeRow = edges.ptr<uchar>(currentRow);
        edgeCol = currentColumn / 3;
    }

    uchar* imageRow = image.ptr<uchar>(currentRow);

    std::bitset<8> dataByte;
    char fileByte;
    while (file.get(fileByte))
    {
        dataByte = std::bitset<8>(fileByte);
        size_t j = 0;
        while (j < dataByte.size())
        {
            if (edgeDetectionType != EdgeDetectionType::None)
            {
                if (edgeRow[edgeCol] == 0)
                {
                    currentColumn++;
                    edgeCol = currentColumn / 3;
                    continue;
                }
            }

            if (dataByte[j])
            {
                imageRow[currentColumn] = setBit(imageRow[currentColumn], 0);
            }
            else
            {
                imageRow[currentColumn] = clearBit(imageRow[currentColumn], 0);
            }

            j++;
            currentColumn++;
            edgeCol = currentColumn / 3;

            if (currentColumn >= nCols)
            {
                currentColumn = 0;
                edgeCol = 0;
                currentRow++;
                if (currentRow >= nRows)
                {
                    // Shouldn't happen. For debugging.
                    return StegoStatus::OUT_OF_ROOM;
                }
                else
                {
                    imageRow = image.ptr<uchar>(currentRow);

                    if (edgeDetectionType != EdgeDetectionType::None)
                    {
                        edgeRow = edges.ptr<uchar>(currentRow);
                    }
                }
            }
        }
    }

    currentRow = 0;
    currentColumn = 0;

    return StegoStatus::SUCCESS;
}

StegoStatus Stego::encodePvdData()
{
    currentColumn = currentColumn / 3;
    int channels = this->blueChannel.channels();
    int nRows = this->blueChannel.rows;
    int nCols = this->blueChannel.cols * channels;
    if (this->blueChannel.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    uchar* blueRow;
    uchar* greenRow;
    uchar* redRow;
    size_t numBitsToEmbed;

    size_t fileNameIndex = 0;
    size_t dataByteIndex = 0;
    std::bitset<8> dataByte = std::bitset<8>(this->fileName[fileNameIndex]);
    std::bitset<7> embeddingNumber = std::bitset<7>();
    for (; currentRow < nRows; currentRow++)
    {
        if (fileNameIndex >= fileName.size())
        {
            break;
        }

        blueRow = this->blueChannel.ptr<uchar>(currentRow);
        greenRow = this->greenChannel.ptr<uchar>(currentRow);
        redRow = this->redChannel.ptr<uchar>(currentRow);

        for (; currentColumn < nCols; currentColumn += 2)
        {
            // No second pixel pair
            if (currentColumn + 1 >= nCols)
            {
                break;
            }

            if (fileNameIndex >= fileName.size())
            {
                break;
            }

            numBitsToEmbed = blueEmbedding.at<uchar>(currentRow, currentColumn);
            if (numBitsToEmbed)
            {
                for (size_t i = 0; i < numBitsToEmbed; i++)
                {
                    embeddingNumber[i] = dataByte[dataByteIndex];

                    dataByteIndex++;
                    if (dataByteIndex >= dataByte.size())
                    {
                        dataByteIndex = 0;
                        fileNameIndex++;
                        if (fileNameIndex >= fileName.size())
                        {
                            break;
                        }

                        dataByte = std::bitset<8>(this->fileName[fileNameIndex]);
                    }
                }

                embedPvdPair(embeddingNumber, numBitsToEmbed, blueRow);
                embeddingNumber.reset();
            }
            else
            {
                embedPvdOverhead(blueRow);
            }

            if (fileNameIndex >= fileName.size())
            {
                break;
            }

            if (edgeDetectionType == EdgeDetectionType::None)
            {
                numBitsToEmbed = greenEmbedding.at<uchar>(currentRow, currentColumn);
                if (numBitsToEmbed)
                {
                    for (size_t i = 0; i < numBitsToEmbed; i++)
                    {
                        embeddingNumber[i] = dataByte[dataByteIndex];

                        dataByteIndex++;
                        if (dataByteIndex >= dataByte.size())
                        {
                            dataByteIndex = 0;
                            fileNameIndex++;
                            if (fileNameIndex >= fileName.size())
                            {
                                break;
                            }

                            dataByte = std::bitset<8>(this->fileName[fileNameIndex]);
                        }
                    }

                    embedPvdPair(embeddingNumber, numBitsToEmbed, greenRow);
                    embeddingNumber.reset();
                }
                else
                {
                    embedPvdOverhead(greenRow);
                }
            }
            else
            {
                if (numBitsToEmbed != 0 || redEmbedding.at<uchar>(currentRow, currentColumn) != 0)
                {
                    greenRow[currentColumn] = setBit(greenRow[currentColumn], 0);
                }
                else
                {
                    greenRow[currentColumn] = clearBit(greenRow[currentColumn], 0);
                }
            }

            if (fileNameIndex >= fileName.size())
            {
                break;
            }

            numBitsToEmbed = redEmbedding.at<uchar>(currentRow, currentColumn);
            if (numBitsToEmbed)
            {
                for (size_t i = 0; i < numBitsToEmbed; i++)
                {
                    embeddingNumber[i] = dataByte[dataByteIndex];

                    dataByteIndex++;
                    if (dataByteIndex >= dataByte.size())
                    {
                        dataByteIndex = 0;
                        fileNameIndex++;
                        if (fileNameIndex >= fileName.size())
                        {
                            break;
                        }

                        dataByte = std::bitset<8>(this->fileName[fileNameIndex]);
                    }
                }

                embedPvdPair(embeddingNumber, numBitsToEmbed, redRow);
                embeddingNumber.reset();
            }
            else
            {
                embedPvdOverhead(redRow);
            }
        }

        if (fileNameIndex >= fileName.size())
        {
            break;
        }

        currentColumn = 0;
    }

    currentColumn += 2;

    std::ifstream file(this->filePath, std::ios_base::binary);
    char fileByte;
    file.get(fileByte);
    dataByte = std::bitset<8>(fileByte);
    dataByteIndex = 0;

    for (; currentRow < nRows; currentRow++)
    {
        if (file.fail())
        {
            break;
        }

        blueRow = this->blueChannel.ptr<uchar>(currentRow);
        greenRow = this->greenChannel.ptr<uchar>(currentRow);
        redRow = this->redChannel.ptr<uchar>(currentRow);

        for (; currentColumn < nCols; currentColumn += 2)
        {
            // No second pixel pair
            if (currentColumn + 1 >= nCols)
            {
                break;
            }

            if (file.fail())
            {
                break;
            }

            numBitsToEmbed = blueEmbedding.at<uchar>(currentRow, currentColumn);
            if (numBitsToEmbed)
            {
                for (size_t i = 0; i < numBitsToEmbed; i++)
                {
                    embeddingNumber[i] = dataByte[dataByteIndex];

                    dataByteIndex++;
                    if (dataByteIndex >= dataByte.size())
                    {
                        dataByteIndex = 0;
                        file.get(fileByte);
                        if (file.fail())
                        {
                            break;
                        }

                        dataByte = std::bitset<8>(fileByte);
                    }
                }

                embedPvdPair(embeddingNumber, numBitsToEmbed, blueRow);
                embeddingNumber.reset();
            }
            else
            {
                embedPvdOverhead(blueRow);
            }

            if (file.fail())
            {
                break;
            }

            if (edgeDetectionType == EdgeDetectionType::None)
            {
                numBitsToEmbed = greenEmbedding.at<uchar>(currentRow, currentColumn);
                if (numBitsToEmbed)
                {
                    for (size_t i = 0; i < numBitsToEmbed; i++)
                    {
                        embeddingNumber[i] = dataByte[dataByteIndex];
                        dataByteIndex++;

                        if (dataByteIndex >= dataByte.size())
                        {
                            dataByteIndex = 0;
                            file.get(fileByte);
                            if (file.fail())
                            {
                                break;
                            }

                            dataByte = std::bitset<8>(fileByte);
                        }
                    }

                    embedPvdPair(embeddingNumber, numBitsToEmbed, greenRow);
                    embeddingNumber.reset();
                }
                else
                {
                    embedPvdOverhead(greenRow);
                }
            }
            else
            {
                if (numBitsToEmbed != 0 || redEmbedding.at<uchar>(currentRow, currentColumn) != 0)
                {
                    greenRow[currentColumn] = setBit(greenRow[currentColumn], 0);
                }
                else
                {
                    greenRow[currentColumn] = clearBit(greenRow[currentColumn], 0);
                }
            }

            if (file.fail())
            {
                break;
            }

            numBitsToEmbed = redEmbedding.at<uchar>(currentRow, currentColumn);
            if (numBitsToEmbed)
            {
                for (size_t i = 0; i < numBitsToEmbed; i++)
                {
                    embeddingNumber[i] = dataByte[dataByteIndex];

                    dataByteIndex++;
                    if (dataByteIndex >= dataByte.size())
                    {

                        dataByteIndex = 0;
                        file.get(fileByte);
                        if (file.fail())
                        {
                            break;
                        }

                        dataByte = std::bitset<8>(fileByte);
                    }
                }

                embedPvdPair(embeddingNumber, numBitsToEmbed, redRow);
                embeddingNumber.reset();
            }
            else
            {
                embedPvdOverhead(redRow);
            }
        }

        currentColumn = 0;
    }

    file.close();

    Mat mergedChannels[3] = {blueChannel, greenChannel, redChannel};
    Mat mergedImage;
    merge(mergedChannels, 3, mergedImage);

    std::filesystem::create_directory("stego_media");
    std::string mediaName = std::filesystem::path(this->mediaPath).filename().string();
    imwrite(std::filesystem::path("stego_media/" + mediaName), mergedImage);

    return StegoStatus::SUCCESS;
}

/**
 * @brief Stego::getLsbSequentialSize
 * @param image to calculate size for
 * @return The number of bits that can be embedded in the image with sequential lsb
 */
uint32_t Stego::getLsbSequentialSize(Mat image)
{
    uint32_t numPixels = image.total();
    uint32_t numPixelsMinusHeader = numPixels - NUM_HEADER_PIXELS;

    return (numPixelsMinusHeader * LSB_BITS_PER_PIXEL) / BITS_PER_BYTE;
}

/**
 * @brief Stego::getLsbEdgeSize
 * @return The number of bits that can be embedded in the image with lsb in edges
 */
uint32_t Stego::getLsbEdgeSize()
{
    uint32_t numPixels = cv::countNonZero(edgeDetector.getMagnitudes());
    return (numPixels * LSB_BITS_PER_PIXEL) / BITS_PER_BYTE;
}

uint32_t Stego::getPvdSequentialSize(Mat image)
{
    Mat channels[3];
    split(image, channels);

    this->blueChannel = channels[0];
    this->greenChannel = channels[1];
    this->redChannel = channels[2];

    calculatePvdEmbeddings();

    return (sum(this->blueEmbedding)[0] +
            sum(this->greenEmbedding)[0] +
            sum(this->redEmbedding)[0]) / BITS_PER_BYTE;
}

uint32_t Stego::getPvdEdgeSize(cv::Mat image)
{
    Mat channels[3];
    split(image, channels);

    this->blueChannel = channels[0];
    this->greenChannel = channels[1];
    this->redChannel = channels[2];

    calculatePvdEmbeddings();

    const Mat magnitudes = edgeDetector.getMagnitudes();
    int numChannels = magnitudes.channels();
    int nRows = magnitudes.rows;
    int nCols = magnitudes.cols * numChannels;

    const double* magnitudeRow;
    double* angleRow;
    double magnitude;

    size_t channelColumn = currentColumn / 3;
    size_t channelRow = currentRow;
    uint32_t total = 0;
    for (;  channelRow < nRows - 1; channelRow++)
    {
        magnitudeRow = magnitudes.ptr<const double>(channelRow);

        for (; channelColumn < nCols - 1; channelColumn += 2)
        {
            if (channelColumn + 1 >= nCols)
            {
                break;
            }

            if (magnitudeRow[channelColumn] != 0 && magnitudeRow[channelColumn + 1] != 0)
            {
                total += blueEmbedding.at<uchar>(channelRow, channelColumn);
                greenEmbedding.at<uchar>(channelRow, channelColumn) = 0;
                total += redEmbedding.at<uchar>(channelRow, channelColumn);
            }
            else
            {
                blueEmbedding.at<uchar>(channelRow, channelColumn) = 0;
                greenEmbedding.at<uchar>(channelRow, channelColumn) = 0;
                redEmbedding.at<uchar>(channelRow, channelColumn) = 0;
            }
        }

        channelColumn = 0;
    }

    return total / BITS_PER_BYTE;
}

bool Stego::isFileTooLarge(Mat image, uint32_t numFrames)
{
    uint32_t fileSizeBytes = std::filesystem::file_size(this->filePath);
    uint32_t fileNameBytes = this->fileName.size();
    uint32_t imageNumEncodeableBytes = 0;
    if (this->algo == StegoAlgo::LSB && this->edgeDetectionType == EdgeDetectionType::None)
    {
        imageNumEncodeableBytes = getLsbSequentialSize(image);
    }
    else if (this->algo == StegoAlgo::PVD && this->edgeDetectionType == EdgeDetectionType::None)
    {
        imageNumEncodeableBytes = getPvdSequentialSize(image);
    }
    else if (this->algo == StegoAlgo::LSB && this->edgeDetectionType != EdgeDetectionType::None)
    {
        imageNumEncodeableBytes = getLsbEdgeSize();
    }
    else if (this->algo == StegoAlgo::PVD && this->edgeDetectionType != EdgeDetectionType::None)
    {
        imageNumEncodeableBytes = getPvdEdgeSize(image);
    }

    return fileSizeBytes + fileNameBytes > (imageNumEncodeableBytes * numFrames);
}

void Stego::calculatePvdEmbeddings()
{
    this->blueEmbedding = Mat(this->blueChannel.rows, this->blueChannel.cols, CV_8UC1);
    this->greenEmbedding = Mat(this->greenChannel.rows, this->greenChannel.cols, CV_8UC1);
    this->redEmbedding = Mat(this->redChannel.rows, this->redChannel.cols, CV_8UC1);

    size_t channelColumn = currentColumn / 3;
    size_t channelRow = currentRow;
    int channels = this->blueChannel.channels();
    int nRows = this->blueChannel.rows;
    int nCols = this->blueChannel.cols * channels;
    if (this->blueChannel.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    uchar* blueRow;
    uchar* greenRow;
    uchar* redRow;
    int difference;
    size_t numBitsToEmbed;

    for (; channelRow < nRows; channelRow++)
    {
        blueRow = this->blueChannel.ptr<uchar>(channelRow);
        greenRow = this->greenChannel.ptr<uchar>(channelRow);
        redRow = this->redChannel.ptr<uchar>(channelRow);

        for (; channelColumn < nCols; channelColumn += 2)
        {
            // No second pixel pair
            if (channelColumn + 1 >= nCols)
            {
                break;
            }

            difference = std::abs(blueRow[channelColumn] - blueRow[channelColumn + 1]);
            numBitsToEmbed = pvdRangeTable[difference];
            if (numBitsToEmbed > MAX_PVD_BLUE_EMBEDDING)
            {
                numBitsToEmbed = 0;
            }

            blueEmbedding.at<uchar>(channelRow, channelColumn) = numBitsToEmbed;

            difference = std::abs(greenRow[channelColumn] - greenRow[channelColumn + 1]);
            numBitsToEmbed = pvdRangeTable[difference];
            if (numBitsToEmbed > MAX_PVD_GREEN_EMBEDDING)
            {
                numBitsToEmbed = 0;
            }

            greenEmbedding.at<uchar>(channelRow, channelColumn) = numBitsToEmbed;

            difference = std::abs(redRow[channelColumn] - redRow[channelColumn + 1]);
            numBitsToEmbed = pvdRangeTable[difference];
            if (numBitsToEmbed > MAX_PVD_RED_EMBEDDING)
            {
                numBitsToEmbed = 0;
            }
            redEmbedding.at<uchar>(channelRow, channelColumn) = numBitsToEmbed;
        }

        channelColumn = 0;
    }
}

void Stego::embedPvdPair(std::bitset<7> embeddingNumber, size_t numBits, uchar *row)
{
    int difference = std::abs(row[currentColumn] - row[currentColumn + 1]);
    int newDifference = embeddingNumber.to_ulong() + pvdRangeLowerBounds[numBits];
    double embed = std::abs(newDifference - difference);

    int firstValue = row[currentColumn];
    int secondValue = row[currentColumn + 1];

    int newFirstValue = 0;
    int newSecondValue = 0;

    bool bitDiscarded = false;

    std::tie<int, int>(newFirstValue, newSecondValue) = calculateNewPvdPixelPairs(firstValue,
                                                                                  secondValue,
                                                                                  difference,
                                                                                  newDifference,
                                                                                  embed);

    if (newFirstValue < 0 || newFirstValue > 255 || newSecondValue < 0 || newSecondValue > 255)
    {
        if (embeddingNumber[numBits - 1])
        {
            embeddingNumber[numBits - 1] = false;
            bitDiscarded = true;
            newDifference = embeddingNumber.to_ullong() + pvdRangeLowerBounds[numBits];
            embed = std::abs(newDifference - difference);
            std::tie<int, int>(newFirstValue, newSecondValue) = calculateNewPvdPixelPairs(firstValue,
                                                                                          secondValue,
                                                                                          difference,
                                                                                          newDifference,
                                                                                          embed);

            if ((newFirstValue >= 0 && newFirstValue <= 255) && (newSecondValue >= 0 && newSecondValue <= 255))
            {
                row[currentColumn] = newFirstValue;
                row[currentColumn + 1] = newSecondValue;
            }
        }

        if (secondValue >= firstValue && newSecondValue > 255)
        {
            newFirstValue = firstValue - embed;
            newSecondValue = secondValue;
        }
        else if (secondValue < firstValue && newFirstValue > 255)
        {
            newFirstValue = firstValue;
            newSecondValue = secondValue - embed;
        }
        else if (secondValue >= firstValue && newFirstValue < 0)
        {
            newFirstValue = firstValue;
            newSecondValue = secondValue + embed;
        }
        else if (secondValue < firstValue && newSecondValue < 0)
        {
            newFirstValue = firstValue + embed;
            newSecondValue = secondValue;
        }
        else if (secondValue == firstValue && newFirstValue > 255)
        {
            newFirstValue = firstValue - embed;
            newSecondValue = secondValue;
        }
        else if (secondValue == firstValue && newSecondValue < 0)
        {
            newFirstValue = firstValue;
            newSecondValue = secondValue + embed;
        }
    }

    bool firstValueLsbZero = newFirstValue % 2 == 0;
    bool secondValueLsbZero = newSecondValue % 2 == 0;

    if (bitDiscarded)
    {
        if (firstValueLsbZero && secondValueLsbZero)
        {
            newFirstValue++;
        }
        else if (firstValueLsbZero && !secondValueLsbZero)
        {
            newFirstValue++;
        }
        else if (!firstValueLsbZero && secondValueLsbZero)
        {
            if (newFirstValue <= 255 && newSecondValue > 0)
            {
                newSecondValue--;
            }
            else if (newFirstValue < 255 && newSecondValue == 0)
            {
                newFirstValue += 2;
                newSecondValue++;
            }
        }
        else if (!firstValueLsbZero && !secondValueLsbZero)
        {
            newSecondValue--;
        }
    }
    else
    {
        if (firstValueLsbZero && secondValueLsbZero)
        {
            newSecondValue++;
        }
        else if (firstValueLsbZero && !secondValueLsbZero)
        {
            if (newFirstValue >= 0 && newSecondValue < 255)
            {
                newSecondValue++;
            }
            else if (newFirstValue > 0 && newSecondValue == 255)
            {
                newFirstValue -= 2;
                newSecondValue--;
            }
            else if (newFirstValue == 0 && newSecondValue == 255)
            {
                // TODO CHECK THIS
                newFirstValue++;
            }
        }
        else if (!firstValueLsbZero && secondValueLsbZero)
        {
            newFirstValue--;
        }
        else if (!firstValueLsbZero && !secondValueLsbZero)
        {
            newFirstValue--;
        }
    }

    row[currentColumn] = newFirstValue;
    row[currentColumn + 1] = newSecondValue;
}

void Stego::embedPvdOverhead(uchar *row)
{
    uchar newFirstValue = row[currentColumn];
    uchar newSecondValue = row[currentColumn + 1];

    bool firstValueLsbZero = newFirstValue % 2 == 0;
    bool secondValueLsbZero = newSecondValue % 2 == 0;

    if (firstValueLsbZero && secondValueLsbZero)
    {
        newSecondValue++;
    }
    else if (firstValueLsbZero && !secondValueLsbZero)
    {
        if (newFirstValue >= 0 && newSecondValue < 255)
        {
            newSecondValue++;
        }
        else if (newFirstValue > 0 && newSecondValue == 255)
        {
            newFirstValue -= 2;
            newSecondValue--;
        }
        else if (newFirstValue == 0 && newSecondValue == 255)
        {
            // TODO CHECK THIS
            newFirstValue++;
        }
    }
    else if (!firstValueLsbZero && secondValueLsbZero)
    {
        newFirstValue--;
    }
    else if (!firstValueLsbZero && !secondValueLsbZero)
    {
        newFirstValue--;
    }

    row[currentColumn] = newFirstValue;
    row[currentColumn + 1] = newSecondValue;
}

std::pair<int, int> Stego::calculateNewPvdPixelPairs(int firstValue, int secondValue, int difference, int newDifference, double embed)
{
    int newFirstValue = 0;
    int newSecondValue = 0;

    if (firstValue >= secondValue && newDifference > difference)
    {
        newFirstValue = firstValue + static_cast<int>(std::ceil(embed / 2));
        newSecondValue = secondValue - static_cast<int>(std::floor(embed / 2));
    }
    else if (firstValue < secondValue && newDifference > difference)
    {
        newFirstValue = firstValue - static_cast<int>(std::floor(embed / 2));
        newSecondValue = secondValue + static_cast<int>(std::ceil(embed / 2));
    }
    else if (firstValue >= secondValue && newDifference <= difference)
    {
        newFirstValue = firstValue - static_cast<int>(std::ceil(embed / 2));
        newSecondValue = secondValue + static_cast<int>(std::floor(embed / 2));
    }
    else if (firstValue < secondValue && newDifference <= difference)
    {
        newFirstValue = firstValue + static_cast<int>(std::ceil(embed / 2));
        newSecondValue = secondValue - static_cast<int>(std::floor(embed / 2));
    }

    return std::make_pair(newFirstValue, newSecondValue);
}

StegoStatus Stego::decodeHeader(Mat image)
{
    int channels = image.channels();
    int nRows = image.rows;
    int nCols = image.cols * channels;
    if (image.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    uchar* row = image.ptr<uchar>(0);

    // Get algorithm from lsb of blue segment of first pixel
    std::bitset<8> intensity(row[0]);
    if (intensity[0])
    {
        // 1
        this->algo = StegoAlgo::PVD;
    }
    else
    {
        // 0
        this->algo = StegoAlgo::LSB;
    }

    // Get Edge Detection from least 2 significant bits of green segment of first pixel
    intensity = std::bitset<8>(row[1]);
    if (intensity[0] == true && intensity[1] == true)
    {
        this->edgeDetectionType = EdgeDetectionType::Sobel;
    }
    else if (intensity[0] == true && intensity[1] == false)
    {
        this->edgeDetectionType = EdgeDetectionType::Canny;
    }
    else if (intensity[0] == false && intensity[1] == false)
    {
        this->edgeDetectionType = EdgeDetectionType::None;
    }
    else
    {
        return StegoStatus::INVALID_HEADER;
    }

    // Validate encryption status from least significant bit of red segment of first pixel
    intensity = std::bitset<8>(row[2]);
    if (intensity[0] == false && this->bEncrypt)
    {
        return StegoStatus::DATA_NOT_ENCRYPTED;
    }

    if (intensity[0] == true && password == "")
    {
        return StegoStatus::DECRYPTION_FAILED;
    }

    std::bitset<18> fileNameLengthBits;
    size_t bitPos = 0;
    currentColumn = 3;
    for (; currentRow < nRows; currentRow++)
    {
        row = image.ptr<uchar>(currentRow);
        for (; currentColumn < nCols; currentColumn++)
        {
            intensity = std::bitset<8>(row[currentColumn]);
            fileNameLengthBits[bitPos] = intensity[0];
            fileNameLengthBits[bitPos + 1] = intensity[1];

            bitPos += 2;
            if (bitPos >= fileNameLengthBits.size())
            {
                break;
            }
        }

        if (bitPos >= fileNameLengthBits.size())
        {
            break;
        }

        currentColumn = 0;
    }

    this->fileNameLength = fileNameLengthBits.to_ulong();

    std::bitset<36> fileLengthBits;
    bitPos = 0;
    for (; currentRow < nRows; currentRow++)
    {
        row = image.ptr<uchar>(currentRow);
        for (; currentColumn < nCols; currentColumn++)
        {
            intensity = std::bitset<8>(row[currentColumn]);
            fileLengthBits[bitPos] = intensity[0];
            fileLengthBits[bitPos + 1] = intensity[1];

            bitPos += 2;
            if (bitPos >= fileLengthBits.size())
            {
                break;
            }
        }

        if (bitPos >= fileLengthBits.size())
        {
            break;
        }

        currentRow = 0;
    }

    this->fileLength = fileLengthBits.to_ulong();

    return StegoStatus::SUCCESS;
}

StegoStatus Stego::decodeLsb(Mat image)
{
    int channels = image.channels();
    int nRows = image.rows;
    int nCols = image.cols * channels;
    if (image.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    uchar* row = image.ptr<uchar>(currentRow);

    std::bitset<8> dataByte;
    std::bitset<8> intensity;

    Mat edges;
    uchar* edgeRow;
    size_t edgeCol = 0;
    if (edgeDetectionType != EdgeDetectionType::None)
    {
        edges = edgeDetector.getMagnitudes();
        edgeRow = edges.ptr<uchar>(currentRow);
        edgeCol = currentColumn / 3;
    }

    for (size_t i = 0; i < this->fileNameLength; i++)
    {
        dataByte.reset();
        size_t j = 0;
        while (j < dataByte.size())
        {
            if (edgeDetectionType != EdgeDetectionType::None)
            {
                if (edgeRow[edgeCol] == 0)
                {
                    currentColumn++;
                    edgeCol = currentColumn / 3;
                    continue;
                }
            }

            intensity = std::bitset<8>(row[currentColumn]);
            dataByte[j] = intensity[0];

            currentColumn++;
            edgeCol = currentColumn / 3;
            j++;

            if (currentColumn >= nCols)
            {
                currentColumn = 0;
                edgeCol = 0;
                currentRow++;

                if (currentRow >= nRows)
                {
                    // Shouldn't happen. For debugging.
                    return StegoStatus::OUT_OF_ROOM;
                }

                row = image.ptr<uchar>(currentRow);
                if (edgeDetectionType != EdgeDetectionType::None)
                {
                    edgeRow = edges.ptr<uchar>(currentRow);
                }
            }
        }

        this->fileName += static_cast<uchar>(dataByte.to_ulong());
    }

    std::filesystem::create_directory("decoded_files");
    std::filesystem::path filePath = std::filesystem::path("decoded_files/" + this->fileName);
    if (this->bEncrypt)
    {
        filePath = std::filesystem::path(tempEncryptFilePath);
    }

    std::ofstream file(filePath, std::ios_base::binary);

    for (size_t i = 0; i < this->fileLength; i++)
    {
        dataByte.reset();
        size_t j = 0;
        while (j < dataByte.size())
        {
            if (edgeDetectionType != EdgeDetectionType::None)
            {
                if (edgeRow[edgeCol] == 0)
                {
                    currentColumn++;
                    edgeCol = currentColumn / 3;
                    continue;
                }
            }

            intensity = std::bitset<8>(row[currentColumn]);
            dataByte[j] = intensity[0];

            currentColumn++;
            edgeCol = currentColumn / 3;
            j++;

            if (currentColumn >= nCols)
            {
                currentColumn = 0;
                edgeCol = 0;
                currentRow++;
                if (currentRow >= nRows)
                {
                    // Shouldn't happen. For debugging.
                    return StegoStatus::OUT_OF_ROOM;
                }
            }
        }

        file.put(static_cast<uchar>(dataByte.to_ulong()));
    }

    file.flush();
    file.close();

    return StegoStatus::SUCCESS;
}

StegoStatus Stego::decodePvd(cv::Mat image)
{
    currentColumn /= 3;
    Mat colours[3];
    split(image, colours);

    this->blueChannel = colours[0];
    this->greenChannel = colours[1];
    this->redChannel = colours[2];

    int channels = blueChannel.channels();
    int nRows = blueChannel.rows;
    int nCols = blueChannel.cols * channels;
    if (blueChannel.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    uchar* blueRow;
    uchar* greenRow;
    uchar* redRow;

    std::bitset<8> dataByte;
    size_t dataByteIndex = 0;
    uchar firstColourValue;
    uchar secondColourValue;
    uchar rangeDifference;
    std::bitset<8> differenceBinary;
    size_t lowerBound;
    size_t numBitsPerBlock;

    for (; currentRow < nRows; currentRow++)
    {
        if (fileName.length() >= fileNameLength)
        {
            break;
        }

        blueRow = this->blueChannel.ptr<uchar>(currentRow);
        greenRow = this->greenChannel.ptr<uchar>(currentRow);
        redRow = this->redChannel.ptr<uchar>(currentRow);

        for (; currentColumn < nCols; currentColumn+= 2)
        {
            if (currentColumn + 1 >= nCols)
            {
                break;
            }

            if (fileName.length() >= fileNameLength)
            {
                break;
            }

            if (edgeDetectionType != EdgeDetectionType::None)
            {
                if (greenRow[currentColumn] % 2 == 0)
                {
                    continue;
                }
            }

            firstColourValue = blueRow[currentColumn];
            secondColourValue = blueRow[currentColumn + 1];

            if (firstColourValue % 2 == 0)
            {
                firstColourValue++;
            }
            else
            {
                firstColourValue--;
            }

            rangeDifference = std::abs(firstColourValue - secondColourValue);
            numBitsPerBlock = pvdRangeTable[rangeDifference];
            if (numBitsPerBlock <= MAX_PVD_BLUE_EMBEDDING)
            {
                lowerBound = pvdRangeLowerBounds[numBitsPerBlock];
                rangeDifference = rangeDifference - lowerBound;

                differenceBinary = std::bitset<8>(rangeDifference);

                if (blueRow[currentColumn] % 2 != 0)
                {
                    differenceBinary[numBitsPerBlock - 1] = true;
                }

                for (size_t i = 0; i < numBitsPerBlock; i++)
                {
                    dataByte[dataByteIndex] = differenceBinary[i];

                    dataByteIndex++;
                    if (dataByteIndex >= dataByte.size())
                    {
                        dataByteIndex = 0;
                        this->fileName += static_cast<uchar>(dataByte.to_ulong());
                        dataByte.reset();

                        if (fileName.length() >= fileNameLength)
                        {
                            break;
                        }
                    }
                }
            }

            if (fileName.length() >= fileNameLength)
            {
                break;
            }

            if (edgeDetectionType == EdgeDetectionType::None)
            {
                firstColourValue = greenRow[currentColumn];
                secondColourValue = greenRow[currentColumn + 1];

                if (firstColourValue % 2 == 0)
                {
                    firstColourValue++;
                }
                else
                {
                    firstColourValue--;
                }

                rangeDifference = std::abs(firstColourValue - secondColourValue);
                numBitsPerBlock = pvdRangeTable[rangeDifference];
                if (numBitsPerBlock <= MAX_PVD_GREEN_EMBEDDING)
                {
                    lowerBound = pvdRangeLowerBounds[numBitsPerBlock];
                    rangeDifference = rangeDifference - lowerBound;

                    differenceBinary = std::bitset<8>(rangeDifference);

                    if (greenRow[currentColumn] % 2 != 0)
                    {
                        differenceBinary[numBitsPerBlock - 1] = true;
                    }

                    for (size_t i = 0; i < numBitsPerBlock; i++)
                    {
                        dataByte[dataByteIndex] = differenceBinary[i];

                        dataByteIndex++;
                        if (dataByteIndex >= dataByte.size())
                        {
                            dataByteIndex = 0;
                            this->fileName += static_cast<uchar>(dataByte.to_ulong());
                            dataByte.reset();

                            if (fileName.length() >= fileNameLength)
                            {
                                break;
                            }
                        }
                    }
                }

                if (fileName.length() >= fileNameLength)
                {
                    break;
                }
            }

            firstColourValue = redRow[currentColumn];
            secondColourValue = redRow[currentColumn + 1];

            if (firstColourValue % 2 == 0)
            {
                firstColourValue++;
            }
            else
            {
                firstColourValue--;
            }

            rangeDifference = std::abs(firstColourValue - secondColourValue);
            numBitsPerBlock = pvdRangeTable[rangeDifference];
            if (numBitsPerBlock <= MAX_PVD_RED_EMBEDDING)
            {
                lowerBound = pvdRangeLowerBounds[numBitsPerBlock];
                rangeDifference = rangeDifference - lowerBound;

                differenceBinary = std::bitset<8>(rangeDifference);

                if (redRow[currentColumn] % 2 != 0)
                {
                    differenceBinary[numBitsPerBlock - 1] = true;
                }

                for (size_t i = 0; i < numBitsPerBlock; i++)
                {
                    dataByte[dataByteIndex] = differenceBinary[i];

                    dataByteIndex++;
                    if (dataByteIndex >= dataByte.size())
                    {
                        dataByteIndex = 0;
                        this->fileName += static_cast<uchar>(dataByte.to_ulong());
                        dataByte.reset();

                        if (fileName.length() >= fileNameLength)
                        {
                            break;
                        }
                    }
                }
            }
        }

        if (fileName.length() >= fileNameLength)
        {
            break;
        }
    }

    currentColumn += 2;
    std::filesystem::create_directory("decoded_files");
    std::filesystem::path filePath = std::filesystem::path("decoded_files/" + this->fileName);
    if (this->bEncrypt)
    {
        filePath = std::filesystem::path(tempEncryptFilePath);
    }

    std::ofstream file(filePath, std::ios_base::binary);
    uint32_t bytesWritten = 0;

    for (; currentRow < nRows; currentRow++)
    {
        if (bytesWritten >= fileLength)
        {
            break;
        }

        blueRow = this->blueChannel.ptr<uchar>(currentRow);
        greenRow = this->greenChannel.ptr<uchar>(currentRow);
        redRow = this->redChannel.ptr<uchar>(currentRow);

        for (; currentColumn < nCols; currentColumn += 2)
        {
            if (currentColumn + 1 >= nCols)
            {
                break;
            }

            if (bytesWritten >= fileLength)
            {
                break;
            }

            if (edgeDetectionType != EdgeDetectionType::None)
            {
                if (greenRow[currentColumn] % 2 == 0)
                {
                    continue;
                }
            }

            firstColourValue = blueRow[currentColumn];
            secondColourValue = blueRow[currentColumn + 1];

            if (firstColourValue % 2 == 0)
            {
                firstColourValue++;
            }
            else
            {
                firstColourValue--;
            }

            rangeDifference = std::abs(firstColourValue - secondColourValue);
            numBitsPerBlock = pvdRangeTable[rangeDifference];
            if (numBitsPerBlock <= MAX_PVD_BLUE_EMBEDDING)
            {
                lowerBound = pvdRangeLowerBounds[numBitsPerBlock];
                rangeDifference = rangeDifference - lowerBound;

                differenceBinary = std::bitset<8>(rangeDifference);

                if (blueRow[currentColumn] % 2 != 0)
                {
                    differenceBinary[numBitsPerBlock - 1] = true;
                }

                for (size_t i = 0; i < numBitsPerBlock; i++)
                {
                    dataByte[dataByteIndex] = differenceBinary[i];

                    dataByteIndex++;
                    if (dataByteIndex >= dataByte.size())
                    {
                        dataByteIndex = 0;
                        file.put(static_cast<uchar>(dataByte.to_ulong()));
                        bytesWritten++;
                        dataByte.reset();

                        if (bytesWritten >= fileLength)
                        {
                            break;
                        }
                    }
                }
            }

            if (bytesWritten >= fileLength)
            {
                break;
            }

            if (edgeDetectionType == EdgeDetectionType::None)
            {
                firstColourValue = greenRow[currentColumn];
                secondColourValue = greenRow[currentColumn + 1];

                if (firstColourValue % 2 == 0)
                {
                    firstColourValue++;
                }
                else
                {
                    firstColourValue--;
                }

                rangeDifference = std::abs(firstColourValue - secondColourValue);
                numBitsPerBlock = pvdRangeTable[rangeDifference];
                if (numBitsPerBlock <= MAX_PVD_GREEN_EMBEDDING)
                {
                    lowerBound = pvdRangeLowerBounds[numBitsPerBlock];
                    rangeDifference = rangeDifference - lowerBound;

                    differenceBinary = std::bitset<8>(rangeDifference);

                    if (greenRow[currentColumn] % 2 != 0)
                    {
                        differenceBinary[numBitsPerBlock - 1] = true;
                    }

                    for (size_t i = 0; i < numBitsPerBlock; i++)
                    {
                        dataByte[dataByteIndex] = differenceBinary[i];

                        dataByteIndex++;
                        if (dataByteIndex >= dataByte.size())
                        {
                            dataByteIndex = 0;
                            file.put(static_cast<uchar>(dataByte.to_ulong()));
                            dataByte.reset();
                            bytesWritten++;

                            if (bytesWritten >= fileLength)
                            {
                                break;
                            }
                        }
                    }
                }

                if (bytesWritten >= fileLength)
                {
                    break;
                }
            }

            firstColourValue = redRow[currentColumn];
            secondColourValue = redRow[currentColumn + 1];

            if (firstColourValue % 2 == 0)
            {
                firstColourValue++;
            }
            else
            {
                firstColourValue--;
            }

            rangeDifference = std::abs(firstColourValue - secondColourValue);
            numBitsPerBlock = pvdRangeTable[rangeDifference];
            if (numBitsPerBlock <= MAX_PVD_RED_EMBEDDING)
            {
                lowerBound = pvdRangeLowerBounds[numBitsPerBlock];
                rangeDifference = rangeDifference - lowerBound;

                differenceBinary = std::bitset<8>(rangeDifference);

                if (redRow[currentColumn] % 2 != 0)
                {
                    differenceBinary[numBitsPerBlock - 1] = true;
                }

                for (size_t i = 0; i < numBitsPerBlock; i++)
                {
                    dataByte[dataByteIndex] = differenceBinary[i];
                    dataByteIndex++;

                    if (dataByteIndex >= dataByte.size())
                    {
                        dataByteIndex = 0;
                        file.put(static_cast<uchar>(dataByte.to_ulong()));
                        bytesWritten++;
                        dataByte.reset();

                        if (bytesWritten >= fileLength)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }

    file.flush();
    file.close();

    return StegoStatus::SUCCESS;
}
