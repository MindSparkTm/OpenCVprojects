
//This code is owned by Surajit Das and is licensed under Icogs Africa. Any tempering or production of this code without the
//permission of the author is strictly proihibited and can result in capital punishments.
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <fstream>
#include <time.h>
#include <Windows.h>
#include <fstream>
#include <string>
#include "opencv2/contrib/contrib.hpp"
#include <vector>
#include "opencv2/video/background_segm.hpp"



using namespace std;
using namespace cv;


const static int SENSITIVITY_VALUE = 40;
const static int BLUR_SIZE = 10;
bool debugMode;
bool trackingEnabled;
void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
     if  ( event == EVENT_LBUTTONDOWN )
     {
          cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
     }
     else if  ( event == EVENT_RBUTTONDOWN )
     {
          cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
     }
     else if  ( event == EVENT_MBUTTONDOWN )
     {
          cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
     }
     else if ( event == EVENT_MOUSEMOVE )
     {
          cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;

     }
}
void morphOps(Mat &thresh){

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

	erode(thresh,thresh,erodeElement);
	erode(thresh,thresh,erodeElement);


	dilate(thresh,thresh,dilateElement);
	dilate(thresh,thresh,dilateElement);



}


string intToString(int number){
	std::stringstream ss;
	ss << number;
	return ss.str();
}


string getDateTime(){
	
	SYSTEMTIME theTime;
	GetLocalTime(&theTime);

	string dateTime;
	string year = intToString(theTime.wYear);
	
	std::stringstream m;
	m<<std::setfill('0')<<std::setw(2)<< theTime.wMonth;
	string month = m.str();
	//day
	std::stringstream d;
	d<<std::setfill('0')<<std::setw(2)<< theTime.wDay;
	string day = d.str();
	//hour
	std::stringstream hr;
	hr<<setfill('0')<<std::setw(2)<<theTime.wHour;
	string hour = hr.str();
	//minute
	std::stringstream min;
	min<<setfill('0')<<std::setw(2)<<theTime.wMinute;
	string minute = min.str();
	//second
	std::stringstream sec;
	sec<<setfill('0')<<std::setw(2)<<theTime.wSecond;
	string second = sec.str();

	
	dateTime = year + "-" + month + "-" + day + "  " + hour + ":" + minute + ":" + second;

	return dateTime;
}
string getDateTimeForFile(){
	
	SYSTEMTIME theTime;
	GetLocalTime(&theTime);
	string dateTime;

	string year = intToString(theTime.wYear);

	std::stringstream m;
	m<<std::setfill('0')<<std::setw(2)<< theTime.wMonth;
	string month = m.str();

	std::stringstream d;
	d<<std::setfill('0')<<std::setw(2)<< theTime.wDay;
	string day = d.str();

	std::stringstream hr;
	hr<<setfill('0')<<std::setw(2)<<theTime.wHour;
	string hour = hr.str();

	std::stringstream min;
	min<<setfill('0')<<std::setw(2)<<theTime.wMinute;
	string minute = min.str();

	std::stringstream sec;
	sec<<setfill('0')<<std::setw(2)<<theTime.wSecond;
	string second = sec.str();

	
	dateTime = year + "_" + month + "_" + day + "_" + hour + "h" + minute + "m" + second + "s";

	return dateTime;
}


static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') {
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, classlabel;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty()) {
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
}

bool detectMotion(Mat thresholdImage, Mat &cameraFeed){
	bool motionDetected = false;
	Mat temp;
	thresholdImage.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(temp,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE );// retrieves external contours

	if(contours.size()>0)motionDetected=true;
	else motionDetected = false;

	return motionDetected;

}
int main(){
	bool recording = false;
	bool startNewRecording = false;
	int inc=0;
	bool firstRun = true;
	bool motionDetected = false;

	bool pause = false;

	debugMode = false;
	trackingEnabled = true;
	
	Mat frame1,frame2;
	Mat grayImage1,grayImage2;
	Mat differenceImage;
	Mat thresholdImage;
	VideoCapture capture;
	capture.open(0);
	VideoWriter oVideoWriter;//create videoWriter object, not initialized yet
	double dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
	double dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
	//set framesize for use with videoWriter
	Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
	bool cap = true;

	if(!capture.isOpened()){
		cout<<"ERROR ACQUIRING VIDEO FEED\n";
		getchar();
		return -1;
	}

    string username,password;
	cout<<"Welcome Admin"<<endl;
	cout<<"Enter your username and password"<<endl;
	cin>>username>>password;

	if(username=="admin")
	{
		if(password=="password")
		{
			cout<<"Successfully Verified"<<endl;

			while(true){


		capture.read(frame1);
		cv::cvtColor(frame1,grayImage1,COLOR_BGR2GRAY);
		capture.read(frame2);
		cv::cvtColor(frame2,grayImage2,COLOR_BGR2GRAY);
		
		cv::absdiff(grayImage1,grayImage2,differenceImage);
		cv::threshold(differenceImage,thresholdImage,SENSITIVITY_VALUE,255,THRESH_BINARY);
		if(debugMode==true){
			cv::imshow("Difference Image",differenceImage);
			cv::imshow("Threshold Image", thresholdImage);
		}else{
			cv::destroyWindow("Difference Image");
			cv::destroyWindow("Threshold Image");
		}
		cv::blur(thresholdImage,thresholdImage,cv::Size(BLUR_SIZE,BLUR_SIZE));
		cv::threshold(thresholdImage,thresholdImage,SENSITIVITY_VALUE,255,THRESH_BINARY);
		if(debugMode==true){

			imshow("Final Threshold Image",thresholdImage);

		}
		else {
			cv::destroyWindow("Final Threshold Image");
		}

		if(trackingEnabled){

			
			motionDetected = detectMotion(thresholdImage,frame1);

		}else{ 
			motionDetected = false;

		}


		String dtime = getDateTime();
		cv::rectangle(frame1,cv::Rect(0,460,200,20),cv::Scalar(255,255,255),-1);
		cv::putText(frame1,dtime,Point(0,480),1,1,Scalar(0,0,0));
		if(recording){

		
			if(firstRun == true || startNewRecording == true){


				string filename = getDateTimeForFile();
					filename = "C:/Users/user/Documents/Visual Studio 2012/Projects/ConsoleApplication7/" + filename + ".avi";
				cout << "File has been opened for writing: " << filename<<endl;
				
				cout << "Frame Size = " << dWidth << "x" << dHeight << endl;

				oVideoWriter  = VideoWriter(filename, CV_FOURCC('D', 'I', 'V', '3'), 10, frameSize, true);

				if ( !oVideoWriter.isOpened() ) 
				{
					cout << "ERROR: Failed to initialize video writing" << endl;
					getchar();
					return -1;
				}
				//reset our variables to false.
				firstRun = false;
				startNewRecording = false;


			}

			oVideoWriter.write(frame1);
			
			putText(frame1,"REC",Point(0,60),2,2,Scalar(0,0,255),2);


		}



		if(motionDetected){
			putText(frame1,"MOTION DETECTED",cv::Point(0,420),2,2,cv::Scalar(0,255,0));
			recording = true;



		} else recording = false;
		//show our captured frame
		imshow("Frame1",frame1);

		
		switch(waitKey(30)){

		case 27: //'esc' key has been pressed, exit program.

			return 0;
		case 116: //'t' has been pressed. this will toggle tracking
			trackingEnabled = !trackingEnabled;
			if(trackingEnabled == false) cout<<"Tracking disabled."<<endl;
			else cout<<"Tracking enabled."<<endl;
			break;
		case 100: //'d' has been pressed. this will debug mode
			debugMode = !debugMode;
			if(debugMode == false) cout<<"Debug mode disabled."<<endl;
			else cout<<"Debug mode enabled."<<endl;
			break;
		case 112://'p' has been pressed. this will pause/resume the code.
			pause = !pause;
			if(pause == true){ cout<<"Code paused, press 'p' again to resume"<<endl;
			while (pause == true){
				switch (waitKey()){
					//a switch statement inside a switch statement? Mind blown.
				case 112: 
					//change pause back to false
					pause = false;
					cout<<"Code Resumed"<<endl;
					break;
				}
			}
			}

		case 114://'r' has been pressed.
			
			recording =!recording;

			if (!recording)cout << "Recording Stopped" << endl;

			else cout << "Recording Started" << endl;

			break;

		case 110://'n' has been pressed
			
			startNewRecording = true;
			recording = true;
			cout << "New Recording Started" << endl;
			inc+=1;
			break; 

		}

	}

	return 0;
	


   waitKey(30);
  }



















		
	

	else
	{
		cout<<"Invalid information"<<endl;
		return 0;
	 }
		
	

	

}
  




	//out.close();


}