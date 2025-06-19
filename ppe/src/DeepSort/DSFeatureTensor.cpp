//#include "globalconfig.h"
#include "DSFeatureTensor.h"
#include <iostream>

FeatureTensor* FeatureTensor::instance = NULL;

FeatureTensor* FeatureTensor::getInstance()
{
    if (instance == NULL)
    {
        instance = new FeatureTensor();
    }
    return instance;
}

FeatureTensor::FeatureTensor()
{
    // prepare model:
    bool status = init();
    if (status == false)
    {
        std::cout << "init failed" << std::endl;
        exit(1);
    }
    else
    {
        std::cout << "init succeed" << std::endl;
    }
}

FeatureTensor::~FeatureTensor()
{
}

bool FeatureTensor::init()
{

    

    return true;
}

void FeatureTensor::preprocess(cv::Mat& imageBGR, std::vector<float>& inputTensorValues, size_t& inputTensorSize)
{

    // pre-processing the Image
    //  step 1: Read an image in HWC BGR UINT8 format.
    //  cv::Mat imageBGR = cv::imread(imageFilepath, cv::ImreadModes::IMREAD_COLOR);

    // step 2: Resize the image.
    cv::Mat resizedImageBGR, resizedImageRGB, resizedImage, preprocessedImage;
    cv::resize(imageBGR, resizedImageBGR,
        cv::Size(inputDims_.at(3), inputDims_.at(2)),
        cv::InterpolationFlags::INTER_CUBIC);

    // cv::resize(imageBGR, resizedImageBGR,
    //            cv::Size(64, 128));

    // step 3: Convert the image to HWC RGB UINT8 format.
    cv::cvtColor(resizedImageBGR, resizedImageRGB,
        cv::ColorConversionCodes::COLOR_BGR2RGB);
    // step 4: Convert the image to HWC RGB float format by dividing each pixel by 255.
    resizedImageRGB.convertTo(resizedImage, CV_32F, 1.0 / 255);

    // step 5: Split the RGB channels from the image.
    cv::Mat channels[3];
    cv::split(resizedImage, channels);

    // step 6: Normalize each channel.
    //  Normalization per channel
    //  Normalization parameters obtained from your custom model

    channels[0] = (channels[0] - 0.485) / 0.229;
    channels[1] = (channels[1] - 0.456) / 0.224;
    channels[2] = (channels[2] - 0.406) / 0.225;

    // step 7: Merge the RGB channels back to the image.
    cv::merge(channels, 3, resizedImage);

    // step 8: Convert the image to CHW RGB float format.
    // HWC to CHW
    cv::dnn::blobFromImage(resizedImage, preprocessedImage);
    inputTensorSize = vectorProduct(inputDims_);
    inputTensorValues.assign(preprocessedImage.begin<float>(),
        preprocessedImage.end<float>());
    //std::cout << "11111111111111111111111111:" << inputTensorValues.size() << std::endl;

    // std::cout << "inputTensorSize:" << inputTensorValues.size() << std::endl;
}

bool FeatureTensor::getRectsFeature(const cv::Mat& img, DETECTIONS& d) {
    ncnn::Net net;
    int feature_dim=512;
	std::vector<cv::Mat> mats;
	for(DETECTION_ROW& dbox : d) {
		cv::Rect rc = cv::Rect(int(dbox.tlwh(0)), int(dbox.tlwh(1)),
				int(dbox.tlwh(2)), int(dbox.tlwh(3)));
		rc.x -= (rc.height * 0.5 - rc.width) * 0.5;
		rc.width = rc.height * 0.5;
		rc.x = (rc.x >= 0 ? rc.x : 0);
		rc.y = (rc.y >= 0 ? rc.y : 0);
		rc.width = (rc.x + rc.width <= img.cols? rc.width: (img.cols-rc.x));
		rc.height = (rc.y + rc.height <= img.rows? rc.height:(img.rows - rc.y));

		cv::Mat mattmp = img(rc).clone();
		cv::resize(mattmp, mattmp, cv::Size(64, 128));
		mats.push_back(mattmp);
	}
    int count = mats.size();

    float norm[3] = {0.229, 0.224, 0.225};
    float mean[3] = {0.485, 0.456, 0.406};
    for (int i=0; i<count; i++)
    {
        ncnn::Mat in_net = ncnn::Mat::from_pixels(mats[i].data, ncnn::Mat::PIXEL_BGR2RGB, 64, 128);
        in_net.substract_mean_normalize(mean, norm);

        ncnn::Mat out_net;
        ncnn::Extractor ex = net.create_extractor();
        ex.set_light_mode(true);
        ex.set_num_threads(4);
    //    if (toUseGPU) {  // 消除提示
    //        ex.set_vulkan_compute(toUseGPU);
    //    }
        ex.input("input", in_net);
        ex.extract("output", out_net);

        cv::Mat tmp(out_net.h, out_net.w, CV_32FC1, (void*)(const float*)out_net.channel(0));
        const float* tp = tmp.ptr<float>(0);
        for(int j = 0; j < feature_dim; j++)
        {
            d[i].feature[j] = tp[j];
        }

    }
	return true;
}

void FeatureTensor::tobuffer(const std::vector<cv::Mat>& imgs, uint8* buf)
{
    int pos = 0;
    for (const cv::Mat& img : imgs)
    {
        int Lenth = img.rows * img.cols * 3;
        int nr = img.rows;
        int nc = img.cols;
        if (img.isContinuous())
        {
            nr = 1;
            nc = Lenth;
        }
        for (int i = 0; i < nr; i++)
        {
            const uchar* inData = img.ptr<uchar>(i);
            for (int j = 0; j < nc; j++)
            {
                buf[pos] = *inData++;
                pos++;
            }
        } // end for
    }     // end imgs;
}
void FeatureTensor::test()
{
    return;
}