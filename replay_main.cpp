#include <royale.hpp>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include "CamListener.h"
#include <cxxopts.hpp>

using namespace royale;
using namespace std;
using namespace cv;

CamListener listener;

static void onMouse( int event, int x, int y, int, void* )
{
    if( event != EVENT_LBUTTONDOWN )
        return;
    Vec3f point = listener.xyzMap.at<Vec3f>(y, x);
    uint8_t confidence = listener.confMap.at<uint8_t>(y,x);
	printf("Depth point(cm): x=%.2f\ty=%.2f\tz=%.2f\t Confidence=%.2f\n", 
			point[0]*100, point[1]*100, point[2]*100, (float)confidence*100/255 );
}


int main (int argc, char *argv[])
{
	// Parse options
	cxxopts::Options options("Replay");
	options
	.allow_unrecognised_options()
	.add_options()
	("f", "Record folder", cxxopts::value<string>(), "file_path");
	cout << options.help() << endl;

	cout << "----- PMD Replay -----" << endl;
	cout << "Press 'p' for pause/resume" << endl;
	cout << "Press 'ESC' for exit" << endl << endl;

	auto result = options.parse(argc, argv);

	if (result.count("f") == 0)
	{
		cout << "give a folder name" << endl;
		return 0;
	}
	string file_path = result["f"].as<std::string>();
	vector<string> file_names;

	boost::filesystem::path image_dir(file_path);
	if (is_directory(image_dir))
	{
		boost::filesystem::directory_iterator end_iter;
		for (boost::filesystem::directory_iterator dir_itr(image_dir); dir_itr != end_iter; ++dir_itr) {
			const auto next_path = dir_itr->path().generic_string();
			file_names.push_back(next_path);
		}
	}
	sort(file_names.begin(), file_names.end());

	listener.initialize(224, 171);

	// windows
	namedWindow ("Gray", WINDOW_AUTOSIZE);
	namedWindow ("Depth", WINDOW_AUTOSIZE);
	setMouseCallback( "Depth", onMouse, 0 );

	string path;
	bool stop = false;
	for(int i=0; i< file_names.size();)
	{
		FileStorage fs2(file_names[i], FileStorage::READ);

		fs2["grayImage"] >> listener.grayImage;
		fs2["xyzMap"] >> listener.xyzMap;
		fs2["confMap"] >> listener.confMap;
		listener.processImages();

		int currentKey = waitKey (100) & 255;
		if (currentKey == 'p' || stop)
		{
			stop = true;
			while(1){
				currentKey = waitKey (0) & 255;
				if(currentKey == 83){
					i++;
					break;
				}
				else if(currentKey == 81 && i>0){
					i--;
					break;
				}
				else if(currentKey == 27){
					i = file_names.size();
					break;
				}
				else if(currentKey == 'p'){
					stop = false;
					break;
				}
			}
		}
		if(!stop) i++;


	}

	return 0;
}
