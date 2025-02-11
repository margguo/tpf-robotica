#include <utils/GarbageRecognition.h>

#include <highgui.h>
#include <stdio.h>
#include <string>
#include <utils/contours.h>
#include <utils/histogram.h>
#include <utils/Garbage.h>
#include <utils/MinimalBoundingRectangle.h>

// image preprocessing values
#define THRESHOLD_VALUE 240
#define MORPH_KERNEL_SIZE 2
#define MORPH_DILATE_ITER 2
#define MORPH_ERODE_ITER 2

#define _RED  cvScalar (0, 0, 255, 0)
#define _GREEN cvScalar (0, 255, 0, 0)

//contour filters constants
#define MINCONTOUR_AREA 1000
#define MAXCONTOUR_AREA 10000
#define BOXFILTER_TOLERANCE 0.55
#define MINCONTOUR_PERIMETER 80
#define MAXCONTOUR_PERIMETER 1000
#define CONTOUR_RECTANGULAR_MIN_RATIO 1.2
#define CONTOUR_RECTANGULAR_MAX_RATIO 3.0
#define HIST_S_BINS 8
#define HIST_H_BINS 8
#define HIST_MIN 0.7
#define TIME_THRESHOLD 15 //seconds

namespace utils {

robotapi::ICamera * cam;

std::list<utils::Garbage*> garbages;
time_t lastRequest;

void GarbageRecognition::setCamera(robotapi::ICamera &camera)
{
	cam = &camera;
	lastRequest = time(NULL);
}

bool GarbageRecognition::thereIsGarbage()
{
	this->getGarbageList();
    return garbages.empty();
}

std::list<Garbage*> GarbageRecognition::getGarbageList()
{
    time_t request = time(NULL);
    
    if ( request - lastRequest > TIME_THRESHOLD ){
		lastRequest = request;
		std::string fn ("./ss.jpg");
		cam->saveImage(fn, 85);
	    IplImage * src = cvLoadImage("./ss.jpg",1);
	    IplImage * model = cvLoadImage("./ss.jpg",1);
		garbages = this->garbageList(src,model);
	}
	return garbages;
}

std::list<Garbage*> GarbageRecognition::garbageList(IplImage * src, IplImage * model){

	std::list<Garbage*> garbageList;
	//cvNamedWindow("output",CV_WINDOW_AUTOSIZE);
	//object model

	//image for the histogram-based filter
	//could be a parameter

	utils::Histogram * h = new Histogram(HIST_H_BINS,HIST_S_BINS);
	CvHistogram * testImageHistogram = h->getHShistogramFromRGB(model);

	//~ int frameWidth=cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH);
	//~ int frameHeight=cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT);



	//gets a frame for setting  image size
	//CvSize srcSize = cvSize(frameWidth,frameHeight);
	CvSize srcSize = cvGetSize(src);

	//images for HSV conversion
	IplImage* hsv = cvCreateImage( srcSize, 8, 3 );
	IplImage* h_plane = cvCreateImage( srcSize, 8, 1 );
	IplImage* s_plane = cvCreateImage( srcSize, 8, 1 );
	IplImage* v_plane = cvCreateImage( srcSize, 8, 1 );



	//Image for thresholding
	IplImage * threshImage=cvCreateImage(srcSize,8,1);

	//image for equalization
	IplImage * equalizedImage=cvCreateImage(srcSize,8,1);

	//image for Morphing operations(Dilate-erode)
	IplImage * morphImage=cvCreateImage(srcSize,8,1);

	//image for image smoothing
	IplImage * smoothImage=cvCreateImage(srcSize,8,1);

	//image for contour-finding operations
	IplImage * contourImage=cvCreateImage(srcSize,8,3);

	int frameCounter=1;
	int cont_index=0;

	//convolution kernel for morph operations
	IplConvKernel* element;

	CvRect boundingRect;

	//contours
	CvSeq ** contours;

	//Main loop


	frameCounter++;
//	printf("frame number:%d\n",frameCounter);

	//convert image to hsv
	cvCvtColor( src, hsv, CV_BGR2HSV );
	cvCvtPixToPlane( hsv, h_plane, s_plane, v_plane, 0 );

	//equalize Saturation Channel image
	cvEqualizeHist(s_plane,equalizedImage);

	//threshold the equalized Saturation channel image
	cvThreshold(equalizedImage,threshImage,THRESHOLD_VALUE,255,
	CV_THRESH_BINARY);

	//apply morphologic operations
	element = cvCreateStructuringElementEx( MORPH_KERNEL_SIZE*2+1,
		MORPH_KERNEL_SIZE*2+1, MORPH_KERNEL_SIZE, MORPH_KERNEL_SIZE,
		CV_SHAPE_RECT, NULL);

	cvDilate(threshImage,morphImage,element,MORPH_DILATE_ITER);
	cvErode(morphImage,morphImage,element,MORPH_ERODE_ITER);

	//apply smooth gaussian-filter
	cvSmooth(morphImage,smoothImage,CV_GAUSSIAN,3,0,0,0);

	//get all contours
	contours = myFindContours(smoothImage);

	cont_index=0;
	cvCopy(src,contourImage,0);

	while(contours[cont_index]!=NULL){
		//CvSeq * aContour=contours[cont_index];
		utils::Contours * ct = new Contours(contours[cont_index]);
	printf("F");
		int	pf = ct->perimeterFilter(MINCONTOUR_PERIMETER,MAXCONTOUR_PERIMETER);
			printf("H");
		int raf = ct->rectangularAspectFilter(CONTOUR_RECTANGULAR_MIN_RATIO, CONTOUR_RECTANGULAR_MAX_RATIO);
			printf("J");
		// int af = ct->areaFilter(MINCONTOUR_AREA,MAXCONTOUR_AREA);
		int baf = ct->boxAreaFilter(BOXFILTER_TOLERANCE);
			printf("K");
		int hmf = ct->histogramMatchingFilter(src,testImageHistogram, HIST_H_BINS,HIST_S_BINS,HIST_MIN);
			printf("L");
		//apply filters
		if( pf && raf && baf && hmf	){
				printf("G");
				//if passed filters
				ct->printContour(3,cvScalar(127,127,0,0),
					contourImage);
				printf("G");
				//get contour bounding box
				boundingRect=cvBoundingRect(ct->getContour(),0);
				cvRectangle(contourImage,cvPoint(boundingRect.x,boundingRect.y),
						cvPoint(boundingRect.x+boundingRect.width,
						boundingRect.y+boundingRect.height),
						_GREEN,1,8,0);
				//build garbage List
				printf("G");
				printf(" c %d,%d\n",boundingRect.x,boundingRect.y);

				utils::MinimalBoundingRectangle * r = new utils::MinimalBoundingRectangle(boundingRect.x,
					boundingRect.y,boundingRect.width,boundingRect.height);



				utils::Garbage * aGarbage = new utils::Garbage(r);

				garbageList.push_back(aGarbage);


			}
	printf("G");
		delete ct;
		cont_index++;
	}
	printf("H");
//	cvShowImage("output",contourImage);
//	cvWaitKey(0);
	delete h;

	return garbageList;
}

} /* End of namespace utils */
