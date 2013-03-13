/*
 * ALPR
 * Michał El Fartas, Marcin Biernacki
 * (c) 2012
 */

#include <iostream>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "leptonica/allheaders.h" // Biblioteka do prostej obróbki obrazów
#include "tesseract/baseapi.h" // Główne API Tesseracta (słaba dokumentacja)
using namespace std;
using namespace tesseract;
using namespace cv;
#include <time.h>
#include "RandomColor.h"
#include "SourceImage.h"
#include "PlateFinder.h"




int main(int argc, char **argv)
{

	if(argc < 2)
	{
		cout << "usage: " << argv[0] << " filename" << endl;
		return -1;
	}

	cout << argv[1] << endl;
	SourceImage img(argv[1]);
//	PlateFinder finder(img,0.05,0.3);
	PlateFinder finder(img,1,0);
	finder.execute();

	return 0;

}

