
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include "xmlParser.h"
#include "Result.h"
#include "visionInterface.h"
#include "GarbageAdapter.h"
#include <vector>
using namespace benchmark;

Result *
compareFrameXmlWithFrame(IplImage* frame,Frame* frameXml);

void
makeGlobalResults(list<Result*>  resultList);

Result*
getFrameResults(IplImage * frame);

void
drawCompare(std::list<Cobject*>,std::list<Cobject*>);


GarbageAdapter * ga;

#define _RED  cvScalar (0, 0, 255, 0)
#define _GREEN cvScalar (0, 255, 0, 0)

typedef struct {
	int hit;
	int falseDetections;
	int nObjects;
} stats;

//frame from video
IplImage * videoFrameImg;
IplImage * compareImg;
	

int main(int argc, char** argv)
{	    
	CvCapture* capture;
	cvNamedWindow("compare",CV_WINDOW_AUTOSIZE);
	
	
	
	if (argc < 3) {
        fprintf(stderr, "Usage: %s filename.xml\n video", argv[0]);
        return 1;
    }
    
    if( (capture=cvCreateFileCapture( argv[2]))== 0 ){
		perror("Invalid capture");
		return 1;
	}
    
    GarbageAdapter * ga= new GarbageAdapter();
    //std::list<Frame*> framesList=parseXmlObjects(argv[1]);
    videoInfo vinfo=parseXmlObjects(argv[1]);
    std::list<Frame*> framesList=vinfo.framesList;
    int maxFrameNumber=vinfo.numberOfFrames;
    int nextTestframe=1,h=0,frameStart=0;
    int videoFrameNumber=1;
    bool continueVideo=true;
    
    std::list<Frame*>::iterator frameXml = framesList.begin();
    nextTestframe=(*frameXml)->frameNumber;
    
    std::list<Result*> resultList;
    
    while(h!=frameStart){
		cvQueryFrame(capture);
		h++;
	}
		
    
    
    while((videoFrameImg=cvQueryFrame(capture))!=NULL && videoFrameNumber<maxFrameNumber)
    {
		Result* result=NULL;
		compareImg=cvCloneImage (videoFrameImg);
		
		
		if(videoFrameNumber==nextTestframe){
			//compare FrameXml with FrameImg
			printf(" comparando frame %d",videoFrameNumber);
			result=compareFrameXmlWithFrame(videoFrameImg,*frameXml);
			
			
			++frameXml;
			if(frameXml != framesList.end()) 
				nextTestframe=(*frameXml)->frameNumber;
		}
		else{
			printf("no test info for frame %d\n",videoFrameNumber);
			result=getFrameResults(videoFrameImg);
		}
	
		if(result!=NULL)
			resultList.push_back(result);
		videoFrameNumber++;
		

		
	}
	
	//cvReleaseImage( &videoFrameImg );
	cvReleaseCapture( &capture ); 
	makeGlobalResults(resultList);
    
    
}

Result*
getFrameResults(IplImage * frame)
{
	Result * res= new Result();
	std::list<Cobject*> objects=ga->recognizeObjects(frame);
	
	for (std::list<Cobject*>::iterator it = objects.begin(); it != objects.end(); it++){
		res->addMiss((*it));
	}
	
	return res;
		
}
Result*
compareFrameXmlWithFrame(IplImage* frame,Frame* frameXml)
{
	bool found=false;
	int hit=0,miss=0;
	std::list<Cobject*> objects=ga->recognizeObjects(frame);
	std::list<Cobject*> objectsXml=frameXml->getObjects();
	std::list<int> foundVideoContours;
	Result *aResult=new Result(frameXml);
	vector<int> missCount (objects.size(),0);
	
	drawCompare(objects,objectsXml);
	
	
	printf("(%d vid-%d xml)\n",objects.size(),objectsXml.size());
	
	for (std::list<Cobject*>::iterator itXml = objectsXml.begin(); itXml != objectsXml.end(); itXml++){
		found=false;
		for (std::list<Cobject*>::iterator itVid = objects.begin(); itVid != objects.end(); itVid++){
			if( (*itVid)->isSimilar(*itXml)){
				if(!found){
					printf("found object %d index %d\n",(*itXml)->index,(*itVid)->index);
					aResult->addFound((*itXml)->index);
					found=true;		
					hit++;
					//could check if other detections also are similar to the one in the xml
					//then it would be proper to keep the one which is more similar
				}
			}else{
					//printf("%d no le pego a %d\n",(*itVid)->index,(*itXml)->index);
					missCount[(*itVid)->index]++;
			}
		}
	}
	
	for (std::list<Cobject*>::iterator itVid = objects.begin(); itVid != objects.end(); itVid++)
	{
		if(missCount[(*itVid)->index]==objectsXml.size()){
			printf("index %d, no encontro nada \n",(*itVid)->index);
			aResult->addMiss((*itVid));
			miss++;
		}
	}
	
	printf("hits:%d, miss:%d\n",hit,miss);
	
	
	return aResult;
}

void
makeGlobalResults(list<Result*>  resultList){
	stats allFramesStats;
	stats testedFrames;
	
	allFramesStats.nObjects=0;
	allFramesStats.falseDetections=0;
	allFramesStats.hit=0;
	
	for (std::list<Result*>::iterator itRes = resultList.begin(); itRes != resultList.end(); itRes++){
		allFramesStats.hit+=(*itRes)->detectedObjects();
		allFramesStats.falseDetections+=(*itRes)->falsePositives;
		allFramesStats.nObjects+=(*itRes)->nObjects;
	}
	
	printf(" Total number of objects to be recognized %d\n",allFramesStats.nObjects);
	printf(" number of hits %d\n",allFramesStats.hit);
	printf(" number of detected objects that didn't exist %d\n",allFramesStats.falseDetections);
	printf(" False negative detection  %g\n",1 - (allFramesStats.hit)/(double) allFramesStats.nObjects);
	double falseNegativeDetectionProb=allFramesStats.hit==0?(allFramesStats.falseDetections>0?1:0):
		allFramesStats.falseDetections/( (double)allFramesStats.hit + allFramesStats.falseDetections);
	printf(" False positive detection  %g\n",falseNegativeDetectionProb);
}

void drawCompare(std::list<Cobject*> objects,std::list<Cobject*> objectsXml){
	Cobject * boundingRect;
	
	for (std::list<Cobject*>::iterator itXml = objectsXml.begin(); itXml != objectsXml.end(); itXml++){
		boundingRect=(*itXml);
		cvRectangle(compareImg,cvPoint(boundingRect->x,boundingRect->y),
								cvPoint(boundingRect->x+boundingRect->w,
								boundingRect->y+boundingRect->h),
								_GREEN,1,8,0);
	}
	for (std::list<Cobject*>::iterator itVid = objects.begin(); itVid != objects.end(); itVid++){
		boundingRect=(*itVid);
		cvRectangle(compareImg,cvPoint(boundingRect->x,boundingRect->y),
								cvPoint(boundingRect->x+boundingRect->w,
								boundingRect->y+boundingRect->h),
								_RED,1,8,0);
		std::vector<int> centroid=(*itVid)->getCentroid();
		
		cvCircle(compareImg,cvPoint(centroid[0],centroid[1]), 5, _RED, 1);

		
	}
	cvShowImage("compare",compareImg);
	cvWaitKey(1000/20);
}
