// Emrah YILDIRIM
// CV HW01 - Find The Brightest Area and maybe second, third

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdio>
#include <limits.h> // to get INT_MAX, to protect against overflow
#include "opencv2/opencv.hpp"

#ifdef LINUX
#include <sys/time.h>
#endif

using namespace cv;
using namespace std;

#define NORMALIZE_THRESH    10
#define NORMALIZE_STEP      10
#define STEP_SIZE           15
#define RECT_HEIGHT         30
#define RECT_WIDTH          30


void convertGrayScale(const Mat& input, Mat& out);
int calculateRectBrightness(Mat& image, Rect roi);
void fillWithZeros(Mat& img, Rect roi);

pair< Rect, int> findBrightestRect(Mat& image);
pair< Rect, int > normalizeTheBrightest(Mat& img, Rect roi);
pair< Rect, int > normalizeXDimReverse(Mat& img, Rect roi);
pair< Rect, int > normalizeYDimReverse(Mat& img, Rect roi);
pair< Rect, int > normalizeYDim(Mat& img, Rect roi);
pair< Rect, int > normalizeXDim(Mat& img, Rect roi);


int main()
{
    VideoCapture capture;
    Mat frame;
    Mat out;
    pair< Rect, int > p;

    long int counter = 0;
    double sec;
    double fps;
    time_t start, end;
    stringstream ss;

    capture.open(0);
#ifdef LINUX
    time(&start);
#endif

    for (size_t i = 0; true; i++)
    {
        capture >> frame;
        convertGrayScale(frame, out);

        p = findBrightestRect(out);
        p = normalizeTheBrightest(out, p.first);
        rectangle(frame, p.first, Scalar(0, 255, 0));
        circle(frame, Point(p.first.tl().x + (p.first.br().x - p.first.tl().x)/2, p.first.tl().y + (p.first.br().y - p.first.tl().y)/2), (p.first.br().x - p.first.tl().x) / 2, Scalar(0, 255, 0), 2);

        if (p.second > 5)
        {
            fillWithZeros(out, p.first);

            p = findBrightestRect(out);
            p = normalizeTheBrightest(out, p.first);
            rectangle(frame, p.first, Scalar(0, 0, 255));

            fillWithZeros(out, p.first);

            p = findBrightestRect(out);
            p = normalizeTheBrightest(out, p.first);
            rectangle(frame, p.first, Scalar(255, 0, 0));
        }

#ifdef LINUX
        counter++;
        time(&end);
        sec = difftime(end, start);
        fps = counter / sec;

        if (counter == (INT_MAX - 1000))
            counter = 0;
#endif

        ss << "FPS : " << fps;
        putText(frame, ss.str(), Point(10, 15), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 2);
        ss.str("");
        imshow("Brightest Spot", frame);
        if (waitKey(30) >= 0) break;
    }

    destroyWindow("Brightest Spot");
    capture.release();

    return 0;
}

void fillWithZeros(Mat& img, Rect roi)
{
    uchar *p = img.data;
    int r_begin = roi.tl().y, r_end = roi.br().y;
    int c_begin = roi.tl().x, c_end = roi.br().x;

    for (int i = r_begin; i < r_end; ++i)
        for (int j = c_begin; j < c_end; ++j)
        {
            //img.at<uchar>(i, j) = 0;
            p[i*img.cols + j] = 0;
        }
}


pair< Rect, int > normalizeTheBrightest(Mat& img, Rect roi)
{
    pair< Rect, int> p1 = normalizeXDimReverse(img, roi);

    pair< Rect, int> p2 = normalizeYDim(img, p1.first);

    pair< Rect, int> p3 = normalizeXDim(img, p2.first);

    return normalizeYDimReverse(img, p3.first);
}


pair< Rect, int > normalizeYDim(Mat& img, Rect roi)
{
    int firstBright = calculateRectBrightness(img, roi);
    int result = 0;
    Rect newRect = roi;
    int tl_x = newRect.tl().x;
    int tl_y = newRect.tl().y;
    int br_x = newRect.br().x;
    int br_y = newRect.br().y;

    while (br_y < img.rows)
    {
        br_y + NORMALIZE_STEP > img.rows ? br_y = img.rows : br_y += NORMALIZE_STEP;
        newRect = Rect(tl_x, tl_y, br_x - tl_x, br_y - tl_y);

        result = calculateRectBrightness(img, newRect);
        if (result > firstBright - NORMALIZE_THRESH)
            roi = newRect;
        else
            break;
    }

    return pair< Rect, int >(roi, result);

}

pair< Rect, int > normalizeXDim(Mat& img, Rect roi)
{
    int firstBright = calculateRectBrightness(img, roi);
    int result = 0;
    Rect newRect = roi;
    int tl_x = newRect.tl().x;
    int tl_y = newRect.tl().y;
    int br_x = newRect.br().x;
    int br_y = newRect.br().y;

    while (br_x < img.cols)
    {
        br_x + NORMALIZE_STEP > img.cols ? br_x = img.cols : br_x += NORMALIZE_STEP;
        newRect = Rect(tl_x, tl_y, br_x - tl_x, br_y - tl_y);

        result = calculateRectBrightness(img, newRect);

        if (result > firstBright - NORMALIZE_THRESH)
            roi = newRect;
        else
            break;
    }

    return pair< Rect, int >(roi, result);

}

pair< Rect, int > normalizeXDimReverse(Mat& img, Rect roi)
{
    int firstBright = calculateRectBrightness(img, roi);
    int result = 0;
    Rect newRect = roi;
    int tl_x = newRect.tl().x;
    int tl_y = newRect.tl().y;
    int br_x = newRect.br().x;
    int br_y = newRect.br().y;

    while (tl_x > 0)
    {
        tl_x - NORMALIZE_STEP < 0 ? tl_x = 0 : tl_x -= NORMALIZE_STEP;
        newRect = Rect(tl_x, tl_y, br_x - tl_x, br_y - tl_y);

        result = calculateRectBrightness(img, newRect);

        if (result > firstBright - NORMALIZE_THRESH)
            roi = newRect;
        else
            break;
    }

    return pair< Rect, int >(roi, result);

}

pair< Rect, int > normalizeYDimReverse(Mat& img, Rect roi)
{
    int firstBright = calculateRectBrightness(img, roi);
    int result = 0;
    Rect newRect = roi;
    int tl_x = newRect.tl().x;
    int tl_y = newRect.tl().y;
    int br_x = newRect.br().x;
    int br_y = newRect.br().y;

    while (tl_y >= 0)
    {
        tl_y - NORMALIZE_STEP < 0 ? tl_y = -1 : tl_y -= NORMALIZE_STEP-1;
        newRect = Rect(tl_x, tl_y, br_x - tl_x, br_y - tl_y+1);

        result = calculateRectBrightness(img, newRect);
        if (result > firstBright - NORMALIZE_THRESH)
            roi = newRect;
        else
            break;
    }

    return pair< Rect, int >(roi, result);

}


pair< Rect, int> findBrightestRect(Mat& image)
{
    int max = -1;
    int current;
    Rect r;

    for (size_t i = 0; i < image.rows; i += STEP_SIZE)
    {
        for (size_t j = 0; j < image.cols; j += STEP_SIZE)
        {
            Rect roi(j, i, RECT_WIDTH, RECT_HEIGHT);

            current = calculateRectBrightness(image, roi);
            if (current > max)
            {
                max = current;
                r = roi;
            }
        }
    }

    return pair< Rect, int >(r, max);
}

int calculateRectBrightness(Mat& image, Rect roi)
{
    double sum = 0;
    uchar *p = image.data;
    int r_begin = roi.tl().y, r_end = roi.br().y;
    int c_begin = roi.tl().x, c_end = roi.br().x;

    for (size_t i = r_begin; i < r_end; i++)
        for (size_t j = c_begin; j < c_end; j++)
        {
            //sum += image.at<uchar>(i, j);
            sum += p[i*image.cols + j];
        }

    return (sum / (roi.height * roi.width));
}

void convertGrayScale(const Mat& input, Mat& out)
{
    out = Mat::zeros(Size(input.cols, input.rows), CV_8U);
    uchar *inp_p = input.data;
    uchar *out_p = out.data;
    /*
    for (int i = 0; i < input.rows * input.cols-3; i++)
    *(out_p++) = *(inp_p++) * 0.59 + *(inp_p++) * 0.3 + *(inp_p++) * 0.11;
    */
    for (int i = 0; i < input.rows; ++i)
        for (int j = 0; j < input.cols; ++j)
            out.at<uchar>(i, j) = input.at<Vec3b>(i, j)[0] * 0.59 + input.at<Vec3b>(i, j)[0] * 0.3 + input.at<Vec3b>(i, j)[0] * 0.11;
}



