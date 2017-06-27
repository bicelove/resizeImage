#include <stdio.h>  // for snprintf
#include <string>
#include <vector>
#include <iostream>
#include <sstream> // stringstream
#include <fstream> // NOLINT (readability /streams)
#include <utility> // Create key-value pair (there could be not used)
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <unistd.h>
#include <time.h>
#include <opencv/cv.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

//dirs to save predict patch logo results
int mkDir(
	const string									keyfile,			//[In]: Input dictionary path
	const string									resultPath)			//[In]: Input save results path
{
	string line;
	string IDdict = keyfile + "dirList.txt";

	/**************************************/	
	std::ifstream IDifs(IDdict);
	if(!IDifs){
		cout<<"[Open dictionary failed:]"<<IDdict;
		return -1;
	}
	
	while(getline(IDifs, line)){
		if(line.length() != 0){
			std::string foldname = resultPath + line;
			//cout<<foldname<<endl;
			if(access(foldname.c_str(), 0) == -1 && mkdir(foldname.c_str(),0777))
				std::cout<<"create FIRST file bag failed!"<<endl;
		}
	}	
	
	IDifs.close();
	
	return 0;
}

//
int getStringIDFromFilePath(
	const string							imageString,
	string									&imageID,
	string									&rectLabel)
{
	string imageIDTemp;
	int sour_pos, postfix_pos;
	string sour_name;
	int rectLabelPos;
	string rectLabelString, rectLabelNew;

	//
	sour_pos 	= imageString.find_last_of('/');
	//cout<<"sour_pos = "<<sour_pos<<endl;	
	sour_name	= imageString.substr(sour_pos + 1);
	postfix_pos = sour_name.find_last_of('.');
	imageIDTemp = sour_name.erase(postfix_pos, 4);  		

	//labelName
	rectLabelString = imageString.substr(0, sour_pos);	
	rectLabelPos = rectLabelString.find_last_of('/');
	rectLabel = rectLabelString.substr(rectLabelPos + 1);		
	
	//combine the same labels (eg. Armani_1 & Armani_2)
	sour_pos = rectLabel.find_last_of('_');
	rectLabelString = rectLabel.substr(sour_pos + 1);
	if(rectLabelString == "1" || rectLabelString == "2" ){
		rectLabelNew = rectLabel.substr(0, sour_pos);
		rectLabel = rectLabelNew;		
	}

	imageID  = imageIDTemp;

	return 0;
}

//
int ReadImageToResize(
	const Mat  									cv_img_origin,
   	const int 									l_side, 
   	Mat  										&cv_img)
{		
	int height = cv_img_origin.rows;
	int width  = cv_img_origin.cols;

	int long_side, short_side;
	if(height > width){
		long_side = height;
		short_side = width;
	}else{
		long_side = width;
		short_side = height;
	}
	/*
	if(long_side/short_side >= 10)  
		return 0;
	*/
	
	if ( long_side <= l_side ){
		cv_img = cv_img_origin;
	}else if (long_side == height ) {
		cv::resize(cv_img_origin, cv_img, cv::Size((int)((l_side*width)/height), l_side));
	}else if(long_side == width){
		cv::resize(cv_img_origin, cv_img, cv::Size(l_side, (int)((l_side*height)/width)));
	}

	return 0;
}

//
int saveImg(
	const string							imageString,
	const string 							svPath,
	const Mat	 							img,
	const long long 						name)
{

	if(!img.data){
		cout<<"[Fail to crop logo image patch!! loadImgPath:]"<<imageString<<endl;
		return -1;
	}	
		
	int nRet = 0;
	string imageID, rectLabel;
	char window_Name[500];

	//imageID
	nRet = getStringIDFromFilePath(imageString, imageID, rectLabel);
	if(nRet != 0){
		cout<<"[image name parse error!]"<<endl;
		return -1;
	}

	/********************** save crops ***************************/
	sprintf(window_Name,"%s/%s/%lld.jpg", svPath.c_str(), rectLabel.c_str(), name); 
	//cout<<"window name :"<<window_Name<<endl;
	imwrite( window_Name, img);

	return 0;
}

//
int imgResize(
	const string 									imgID,
	const string 									svPath)
{
	FILE* fpQueryList = fopen(imgID.c_str(), "r");
	if(NULL == fpQueryList)
	{
		cout<<"cannot open "<<imgID<<endl;
		return -1;
	}

	int nRet = 0;
	char buff[1000];
	
	int l_side = 150;
	int numImg;
	long long imgName;

	numImg = 0;
	imgName = 20170405000000;
	while(fgets(buff, 1000 ,fpQueryList) != NULL)
	{	
		imgName++;
		char *pstr = strtok(buff, "\n");
		//cout<<"pstr:"<<pstr<<endl;

		Mat cv_img_origin = cv::imread(pstr, 1);
		if(!cv_img_origin.data) {
			std::cout << "Could not open or find the file :" << pstr <<endl;
			continue;
		}		

		//
		Mat cv_img; 
		nRet = ReadImageToResize(cv_img_origin, l_side, cv_img);
		cv_img_origin.release();
		if(nRet != 0){
			cv_img.release();
			continue;
		}
		//cout<<"cv_img_origin.cols = "<<cv_img_origin.cols<<endl;
		//cout<<"cv_img_origin.rows = "<<cv_img_origin.rows<<endl;

		nRet = saveImg(pstr, svPath, cv_img, imgName);
		cv_img.release();
		if(nRet != 0){			
			continue;
		}

		numImg++;
		if(numImg % 100 == 0){
			cout<<"the "<<numImg<<"th image end!"<<endl;		}
		
	}
	
	fclose(fpQueryList);
	
	return 0;
}

//
int main(int argc, char** argv) {

	const int num_required_args = 3;
	if( argc < num_required_args ){
	    cout<<
	    "This program extracts features of an image and predict its label and score.\n"
	    "Usage: Demo_mainboby szQueryList outputList\n";
	    return 1;
  }	
	
	/***********************************Init**********************************/
	string imgList = argv[1];		
	string svPath = argv[2];	
	string keyfilePath = argv[3];
	
	int nRet = 0;		
	/**************************** getAllDataQueryList *************************/
	nRet = mkDir(keyfilePath, svPath);
	if(nRet != 0){
		cout<<"fail to getQueryList!"<<endl;
		return -1;
	}

	nRet = imgResize(imgList, svPath);
	if(nRet != 0){
		cout<<"fail to getQueryList!"<<endl;
		return -1;
	}
	
	cout<<"deal end!"<<endl;
	
	return 0;
}

