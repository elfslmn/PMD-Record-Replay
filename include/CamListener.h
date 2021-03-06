
//
// Created by esalman17 on 5.10.2018.
//

#include <royale/LensParameters.hpp>
#include <royale/IDepthDataListener.hpp>
#include "opencv2/opencv.hpp"
#include <mutex>

using namespace royale;
using namespace std;
using namespace cv;

class CamListener : public royale::IDepthDataListener {

const int MARGIN = 10;

public:
    // Constructors
    CamListener();
    void initialize(uint16_t width, uint16_t height);
    void setLensParameters (LensParameters lensParameters);
    void startRecord(string destFolder);
    void stopRecord();
    void processImages();
        
    Mat xyzMap, confMap;
    Mat grayImage;
    Mat depthImage8, grayImage8;
    Mat cameraMatrix, distortionCoefficients;
    
    bool isRecording = false;

private:
    // Private methods
    void onNewData (const DepthData *data);
    void updateImages(const DepthData* data, Mat & depth, Mat & gray, int min_confidence = 0, bool flip=false);
    bool visualizeImage(const Mat & src, Mat & dest, float resize_factor = 1.0, bool color = false);
    void updateMaps(const DepthData* data, bool flip=false);
    bool saveFrame(int frame);
    
    // Private variables
    uint16_t cam_width, cam_height;
    mutex flagMutex;
    
    int frame = 0;
    string recordFolder = "record\\";

};



