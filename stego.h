#ifndef STEGO_H
#define STEGO_H

#include <string>
#include <opencv4/opencv2/opencv_modules.hpp>
#include <cstdint>
#include <bitset>
#include <opencv4/opencv2/opencv.hpp>
#include "StegoAlgo.h"
#include "EdgeDetectionType.h"
#include "StegoStatus.h"
#include "edgedetection.h"
#include <cryptopp/cryptlib.h>
#include <cryptopp/default.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

class Stego
{
public:
    Stego(std::string filePath, std::string mediaPath, std::string algo, std::string edgeDetection, bool bEncrypt, std::string password);
    Stego(std::string mediaPath, bool bEncrypt, std::string password);
    ~Stego();

    StegoStatus EncodeImage();
    StegoStatus DecodeImage();

    StegoStatus EncodeVideo();
    StegoStatus DecodeVideo();

    StegoStatus EncryptFile();
    StegoStatus DecryptFile();

private:
    static constexpr std::array<size_t, 256> pvdRangeTable = [] {
        std::array<size_t, 256> result {};
        for (size_t i = 0; i < 8; ++ i)
        {
            result[i] = 2;
        }

        for (size_t i = 8; i < 16; ++ i)
        {
            result[i] = 3;
        }

        for (size_t i = 16; i < 32; ++ i)
        {
            result[i] = 4;
        }

        for (size_t i = 32; i < 64; ++ i)
        {
            result[i] = 5;
        }

        for (size_t i = 64; i < 128; ++ i)
        {
            result[i] = 6;
        }

        for (size_t i = 128; i < 256; ++ i)
        {
            result[i] = 7;
        }

        return result;
    }();

    static constexpr std::array<size_t, 8> pvdRangeLowerBounds = [] {
        std::array<size_t, 8> result {};
        result[2] = 0;
        result[3] = 8;
        result[4] = 16;
        result[5] = 32;
        result[6] = 64;
        result[7] = 128;

        return result;
    }();


    std::string filePath;
    std::string fileName;
    std::string mediaPath;

    StegoAlgo algo;
    EdgeDetectionType edgeDetectionType;

    size_t currentRow;
    size_t currentColumn;

    uint16_t fileNameLength;
    uint32_t fileLength;
    bool bEncrypt;
    std::string password;

    cv::Mat greenChannel;
    cv::Mat blueChannel;
    cv::Mat redChannel;

    cv::Mat greenEmbedding;
    cv::Mat blueEmbedding;
    cv::Mat redEmbedding;

    EdgeDetection edgeDetector;

    StegoStatus encodeLsb(cv::Mat image);
    StegoStatus encodePvd();
    StegoStatus encodeHeader(cv::Mat image);
    StegoStatus encodeLsbFileName(cv::Mat image);
    StegoStatus encodePvdFileName();
    StegoStatus encodeLsbFile(cv::Mat image, std::ifstream& file, std::bitset<8>& dataByte, size_t& dataByteIndex);
    StegoStatus encodePvdFile(std::ifstream& file, std::bitset<8>& dataByte, size_t& dataByteIndex);
    uint32_t getLsbSequentialSize(cv::Mat image);
    uint32_t getLsbEdgeSize();
    uint32_t getPvdSequentialSize(cv::Mat image);
    uint32_t getPvdEdgeSize(cv::Mat image);
    bool isFileTooLarge(cv::Mat image, uint32_t numFrames = 1);
    void calculatePvdEmbeddings();
    void embedPvdPair(std::bitset<7> embeddingNumber, size_t numBits, uchar* row);
    void embedPvdOverhead(uchar* row);
    std::pair<int, int> calculateNewPvdPixelPairs(int firstValue, int secondValue, int difference, int newDifference, double embed);

    StegoStatus decodeHeader(cv::Mat image);
    StegoStatus decodeLsbFileName(cv::Mat image);
    StegoStatus decodeLsbFile(cv::Mat image, std::ofstream& file, size_t& bytesWritten, std::bitset<8>& dataByte, size_t& dataByteIndex);
    StegoStatus decodePvdFileName(cv::Mat image);
    StegoStatus decodePvdFile(cv::Mat image, std::ofstream& file, size_t& bytesWritten, std::bitset<8>& dataByte, size_t& dataByteIndex);
};

#endif // STEGO_H
