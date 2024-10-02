#ifndef EDGEDETECTION_H
#define EDGEDETECTION_H

#include "StegoStatus.h"
#include "EdgeDetectionType.h"
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>


class EdgeDetection
{
public:
    EdgeDetection();
    StegoStatus DetectEdges(cv::Mat image, EdgeDetectionType edgeDetectionType);
    cv::Mat GetMagnitudes() const;

private:
    cv::Mat magnitudes;
    cv::Mat angles;
    cv::Mat edgeStrengths;

    StegoStatus canny(cv::Mat image);
    StegoStatus sobel(cv::Mat image);
    void calculatePixelMagnitudes(cv::Mat imageX, cv::Mat imageY);
    void gradientMagnitude();
    void thresholding();
    void hysteresis();
};

#endif // EDGEDETECTION_H
