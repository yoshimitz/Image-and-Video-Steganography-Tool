#include "edgedetection.h"
#include <qdebug.h>

const size_t CANNY_LOWER_THRESHOLD = 64;
const size_t CANNY_UPPER_THRESHOLD = 128;

const uchar WEAK_EDGE = 0;
const uchar STRONG_EDGE = 1;

double roundAngle(double angle)
{
    if (angle >= 0 && angle < 22.5)
    {
        angle = 0;
    }
    else if (angle > 22.5 && angle < 67.5)
    {
        angle = 45;
    }
    else if (angle > 67.5 && angle < 112.5)
    {
        angle = 90;
    }
    else if (angle > 112.5 && angle < 157.5)
    {
        angle = 135;
    }
    else if (angle > 157.5 && angle <= 180)
    {
        angle = 0;
    }
    else
    {
        Q_ASSERT_X(false, "roundAngle", "Invalid angle for rounding");
    }

    return angle;
}

EdgeDetection::EdgeDetection() {}

StegoStatus EdgeDetection::DetectEdges(cv::Mat image, EdgeDetectionType edgeDetectionType)
{
    StegoStatus status = StegoStatus::SUCCESS;

    if (edgeDetectionType == EdgeDetectionType::Canny)
    {
        status = canny(image);
    }
    else if (edgeDetectionType == EdgeDetectionType::Sobel)
    {
        status = sobel(image);
    }
    else
    {
        status = StegoStatus::INVALID_HEADER;
    }

    // namedWindow("test", cv::WINDOW_AUTOSIZE ); // Create a window for display.
    // cv::imshow("test", magnitudes);
    // cv::waitKey(0);

    return status;
}

cv::Mat EdgeDetection::GetMagnitudes() const
{
    return magnitudes;
}

StegoStatus EdgeDetection::sobel(cv::Mat image)
{
    cv::Mat gaussianKernel = (cv::Mat_<double>(5, 5) << 1,  4,  6,  4, 1,
                                                        4, 16, 24, 16, 4,
                                                        6, 24, 36, 24, 6,
                                                        4, 16, 24, 16, 4,
                                                        1,  4,  6,  4, 1)
                                                        / (double) 256;

    cv::Mat sobelXKernel, sobelYKernel;
    cv::Mat maskedImage, dst, dstX, dstY, dstAbsX, dstAbsY;

    sobelXKernel = (cv::Mat_<double>(3, 3) << -1, 0, 1, -2, 0, 2, -1, 0, 1);
    sobelYKernel = (cv::Mat_<double>(3, 3) << 1, 2, 1, 0, 0, 0, -1, -2, -1);

    cv::flip(sobelXKernel, sobelXKernel, -1);
    cv::flip(sobelYKernel, sobelYKernel, -1);

    cv::bitwise_and(image, 252, maskedImage);

    cv::filter2D(maskedImage, dst, -1, gaussianKernel, cv::Point(-1, -1), 0, cv::BORDER_REFLECT);
    cv::filter2D(dst, dstX, CV_64F, sobelXKernel, cv::Point(-1, -1), 0, cv::BORDER_REFLECT);
    cv::filter2D(dst, dstY, CV_64F, sobelYKernel, cv::Point(-1, -1), 0, cv::BORDER_REFLECT);

    // Scale values back to between 0-255
    convertScaleAbs(dstX, dstAbsX);
    convertScaleAbs(dstY, dstAbsY);

    calculatePixelMagnitudes(dstAbsX, dstAbsY);

    return StegoStatus::SUCCESS;
}

void EdgeDetection::calculatePixelMagnitudes(cv::Mat imageX, cv::Mat imageY)
{
    int channels = imageX.channels();
    int nRows = imageX.rows;
    int nCols = imageX.cols * channels;
    if (imageX.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    uchar* rowX;
    uchar* rowY;
    double* rowMagnitude;
    double* rowAngle;

    double magnitude = 0;

    magnitudes = cv::Mat(imageX.rows, imageX.cols, CV_64FC1, 0.0);
    angles = cv::Mat(imageX.rows, imageX.cols, CV_64FC1, 0.0);
    for (size_t i = 0; i < nRows; i++)
    {
        rowX = imageX.ptr<uchar>(i);
        rowY = imageY.ptr<uchar>(i);
        rowMagnitude = magnitudes.ptr<double>(i);
        rowAngle = angles.ptr<double>(i);

        for (size_t j = 0, k = 0; j < nCols; j+= 3, k++)
        {
            rowMagnitude[k] = sqrt(pow(rowX[j], 2) + pow(rowY[j], 2));
            rowAngle[k] = atan2(rowY[j], rowX[j]) * 180 / M_PI;

            magnitude = sqrt(pow(rowX[j + 1], 2) + pow(rowY[j + 1], 2));
            if (magnitude > rowMagnitude[k])
            {
                rowMagnitude[k] = magnitude;
                rowAngle[k] = atan2(rowY[j + 1], rowX[j + 1]) * 180 / M_PI;
            }

            magnitude = sqrt(pow(rowX[j + 2], 2) + pow(rowY[j + 2], 2));
            if (magnitude > rowMagnitude[k])
            {
                rowMagnitude[k] = magnitude;
                rowAngle[k] = atan2(rowY[j + 2], rowX[j + 2]) * 180 / M_PI;
            }

            if (rowAngle[k] < 0)
            {
                rowAngle[k] += 180;
            }

            rowAngle[k] = roundAngle(rowAngle[k]);

            if (rowMagnitude[k] < 20)
            {
                rowMagnitude[k] = 0;
            }
        }
    }

    convertScaleAbs(magnitudes, magnitudes);
}

void EdgeDetection::gradientMagnitude()
{
    int channels = magnitudes.channels();
    int nRows = magnitudes.rows;
    int nCols = magnitudes.cols * channels;

    uchar* magnitudeRow;
    double* angleRow;

    double magnitude;
    double angle;
    for (size_t i = 1; i < nRows - 1; i++)
    {
        magnitudeRow = magnitudes.ptr<uchar>(i);
        angleRow = angles.ptr<double>(i);

        for (size_t j = 1; j < nCols - 1; j++)
        {
            magnitude = magnitudeRow[j];
            angle = angleRow[j];

            if (angle == 0)
            {
                if (magnitude < magnitudeRow[j - 1] || magnitude < magnitudeRow[j + 1])
                {
                    magnitudeRow[j] = 0;
                }
            }
            else if (angle == 45)
            {
                if (magnitude < magnitudes.ptr<uchar>(i - 1)[j + 1] || magnitude < magnitudes.ptr<uchar>(i + 1)[j - 1])
                {
                    magnitudeRow[j] = 0;
                }
            }
            else if (angle == 90)
            {
                if (magnitude < magnitudes.ptr<uchar>(i - 1)[j] || magnitude < magnitudes.ptr<uchar>(i + 1)[j])
                {
                    magnitudeRow[j] = 0;
                }
            }
            else if (angle == 135)
            {
                if (magnitude < magnitudes.ptr<uchar>(i - 1)[j - 1] || magnitude < magnitudes.ptr<uchar>(i + 1)[j + 1])
                {
                    magnitudeRow[j] = 0;
                }
            }
        }
    }
}

void EdgeDetection::thresholding()
{
    edgeStrengths = cv::Mat(magnitudes.rows, magnitudes.cols, CV_8UC1, 0.0);

    int channels = magnitudes.channels();
    int nRows = magnitudes.rows;
    int nCols = magnitudes.cols * channels;

    uchar* magnitudeRow;
    uchar* edgeStrengthRow;

    double magnitude;
    for (size_t i = 1; i < nRows - 1; i++)
    {
        magnitudeRow = magnitudes.ptr<uchar>(i);
        edgeStrengthRow = edgeStrengths.ptr<uchar>(i);
        for (size_t j = 1; j < nCols - 1; j++)
        {
            magnitude = magnitudeRow[j];
            if (magnitude > CANNY_UPPER_THRESHOLD)
            {
                edgeStrengthRow[j] = STRONG_EDGE;
            }
            else if (magnitude > CANNY_LOWER_THRESHOLD)
            {
                edgeStrengthRow[j] = WEAK_EDGE;
            }
            else
            {
                magnitudeRow[j] = 0;
            }
        }
    }
}

void EdgeDetection::hysteresis()
{
    int channels = magnitudes.channels();
    int nRows = magnitudes.rows;
    int nCols = magnitudes.cols * channels;

    uchar* magnitudeRow;
    uchar* edgeStrengthRow;
    uchar* rowAbove;
    uchar* rowBelow;

    for (size_t i = 1; i < nRows - 1; i++)
    {
        magnitudeRow = magnitudes.ptr<uchar>(i);

        edgeStrengthRow = edgeStrengths.ptr<uchar>(i);
        rowAbove = edgeStrengths.ptr<uchar>(i - 1);
        rowBelow = edgeStrengths.ptr<uchar>(i + 1);

        for (size_t j = 1; j < nCols - 1; j++)
        {
            if (edgeStrengthRow[j] == WEAK_EDGE && magnitudeRow[j] != 0)
            {
                if (rowAbove[j - 1] == STRONG_EDGE || rowAbove[j] == STRONG_EDGE || rowAbove[j + 1] == STRONG_EDGE
                    || edgeStrengthRow[j - 1] == STRONG_EDGE || edgeStrengthRow[j + 1] == STRONG_EDGE
                    || rowBelow[j - 1] == STRONG_EDGE || rowBelow[j] == STRONG_EDGE || rowBelow[j + 1] == STRONG_EDGE)
                {
                    edgeStrengthRow[j] = STRONG_EDGE;
                }
                else
                {
                    magnitudeRow[j] = 0;
                }
            }
        }
    }
}


StegoStatus EdgeDetection::canny(cv::Mat image)
{
    StegoStatus status = sobel(image);
    if (status != StegoStatus::SUCCESS)
    {
        return status;
    }

    gradientMagnitude();
    thresholding();
    hysteresis();

    return status;
}
