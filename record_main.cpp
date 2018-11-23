#include <royale.hpp>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "CamListener.h"
#include <cxxopts.hpp>

using namespace royale;
using namespace std;
using namespace cv;

// this represents the main camera device object
static std::unique_ptr<ICameraDevice> cameraDevice;

int main (int argc, char *argv[])
{
	// Parse options
	cxxopts::Options options("Record");
	options
      .allow_unrecognised_options()
      .add_options()
    ("f", "Record folder", cxxopts::value<string>(), "file_path")
    ("e", "Exposure times", cxxopts::value<int>(), "exp_time");
    cout << options.help() << endl;
    
	cout << "----- PMD Record -----" << endl;
	cout << "Press 'p' for pause/resume" << endl;
	cout << "Press 'r' for record/stop" << endl;
	cout << "Press 'ESC' for exit" << endl << endl;
	
	auto result = options.parse(argc, argv);
	
    CamListener listener;
    std::unique_ptr<ICameraDevice> cameraDevice;
    {
        CameraManager manager;

        // check the number of arguments
        if (argc > 1)
        {
            // if the program was called with an argument try to open this as a file
            cout << "Trying to open : " << argv[1] << endl;
            cameraDevice = manager.createCamera (argv[1]);
        }
        else
        {
            // if no argument was given try to open the first connected camera
            royale::Vector<royale::String> camlist (manager.getConnectedCameraList());
            cout << "Detected " << camlist.size() << " camera(s)." << endl;

            if (!camlist.empty())
            {
                cameraDevice = manager.createCamera (camlist[0]);
            }
            else
            {
                cerr << "No suitable camera device detected." << endl
                     << "Please make sure that a supported camera is plugged in, all drivers are "
                     << "installed, and you have proper USB permission" << endl;
                return 1;
            }

            camlist.clear();
        }
    }
    // the camera device is now available and CameraManager can be deallocated here

    if (cameraDevice == nullptr)
    {
        // no cameraDevice available
        if (argc > 1)
        {
            cerr << "Could not open " << argv[1] << endl;
            return 1;
        }
        else
        {
            cerr << "Cannot create the camera device" << endl;
            return 1;
        }
    }

    // IMPORTANT: call the initialize method before working with the camera device
    auto status = cameraDevice->initialize();
    if (status != CameraStatus::SUCCESS)
    {
        cerr << "Cannot initialize the camera device, error string : " << getErrorString (status) << endl;
        return 1;
    }
    
    royale::Vector<royale::String> opModes;
    status = cameraDevice->getUseCases (opModes);
    if (status != CameraStatus::SUCCESS)
    {
        printf("Failed to get use cases, CODE %d\n", (int) status);
        return 1;
    }
    
    // set an operation mode
    status = cameraDevice->setUseCase (opModes[0]);
    if (status != CameraStatus::SUCCESS)
    {
        printf("Failed to set use case, CODE %d\n", (int) status);
        return 1;
    }
    
	uint16_t cam_width, cam_height;
    status = cameraDevice->getMaxSensorWidth (cam_width);
    if (CameraStatus::SUCCESS != status)
    {
        cerr << "failed to get max sensor width: " << getErrorString (status) << endl;
        return 1;
    }

    status = cameraDevice->getMaxSensorHeight (cam_height);
    if (CameraStatus::SUCCESS != status)
    {
        cerr << "failed to get max sensor height: " << getErrorString (status) << endl;
        return 1;
    }
    listener.initialize(cam_width, cam_height);

	if(result.count("e"))
	{
	   //set exposure mode to manual
	   status = cameraDevice->setExposureMode (ExposureMode::MANUAL);
	   if (status != CameraStatus::SUCCESS)
	   {
		  printf ("Failed to set exposure mode, CODE %d\n", (int) status);
	   }

		//set exposure time
		status = cameraDevice->setExposureTime(result["e"].as<int>());
		if (status != CameraStatus::SUCCESS)
		{
		    printf ("Failed to set exposure time, CODE %d\n", (int) status);
		}
	}
	else
	{
		//set exposure mode to auto
	   status = cameraDevice->setExposureMode (ExposureMode::AUTOMATIC);
	   if (status != CameraStatus::SUCCESS)
	   {
		  printf ("Failed to set exposure mode, CODE %d\n", (int) status);
	   }
	} 

    // retrieve the lens parameters from Royale
    LensParameters lensParameters;
    status = cameraDevice->getLensParameters (lensParameters);
    if (status != CameraStatus::SUCCESS)
    {
        cerr << "Can't read out the lens parameters" << endl;
        return 1;
    }
    listener.setLensParameters (lensParameters);

    // register a data listener
    if (cameraDevice->registerDataListener (&listener) != CameraStatus::SUCCESS)
    {
        cerr << "Error registering data listener" << endl;
        return 1;
    }

    namedWindow ("Gray", WINDOW_AUTOSIZE);
    namedWindow ("Depth", WINDOW_AUTOSIZE);

    // start capture mode
    if (cameraDevice->startCapture() != CameraStatus::SUCCESS)
    {
        cerr << "Error starting the capturing" << endl;
        return 1;
    }
    else cout << "Capture started" << endl;

    int currentKey = 0;
    bool isCapturing;
    string folder = "record";

    while (currentKey != 27)
    {
        // wait until a key is pressed
        currentKey = waitKey (0) & 255;
        if (currentKey == 'p')
        {
        	cameraDevice->isCapturing(isCapturing);
        	 if(isCapturing)
        	 {
        	 	if (cameraDevice->stopCapture() != CameraStatus::SUCCESS)
        			cerr << "Error stopping the capturing" << endl;
        	 	else cout << "Capture stopped" << endl;
        	 }
        	 else
        	 {
        	 	if (cameraDevice->startCapture() != CameraStatus::SUCCESS)
        			cerr << "Error stopping the capturing" << endl;
        		else cout << "Capture started" << endl;
        	 }
        	 
    	}
    	if(currentKey == 'r'){
    		if(listener.isRecording) 
    		{
    			listener.stopRecord();
				ofstream file;
			 	file.open (folder+"/Specs.txt");
			  	file << "Exposure time: " ;
			  	if(result.count("e")) file << result["e"].as<int>() << endl;
			  	else file << "Auto" << endl;
			  	file.close();
    		}
    		else
    		{
    			if (result.count("f")) folder = result["f"].as<std::string>();
    			const int dir_err = system(("mkdir -p "+folder).c_str());
				if (dir_err < 0) printf("Error creating directory\n");
    			else listener.startRecord(folder);
    		}
    	}
    }
	
    // stop capture mode
    if (isCapturing && cameraDevice->stopCapture() != CameraStatus::SUCCESS)
    {
        cerr << "Error stopping the capturing" << endl;
        return 1;
    }
    
    destroyAllWindows();

    return 0;
}
