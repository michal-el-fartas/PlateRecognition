#ifndef SOURCEIMAGE_H_INCLUDED
#define SOURCEIMAGE_H_INCLUDED

class SourceImage
{
#define GRAYSCALE 1
#define COLOR_RGB 2
#define PROCESSED 3
	Mat img_color,img_gray,img_var;
	Mat *img_out;
	vector<Rect> boundRect;
	vector<Rect> plates_rect;
	vector<Mat> plates_img;
	vector<pair<float, float> > sv;
	string name;

	void shear(Mat &img,float vertical,float horizontal)
	{
		int rows=ceil(float(img.rows)*(1+abs(vertical)));
		int cols=ceil(float(img.cols)*(1+abs(horizontal)));
		Mat mid(rows,cols,img.type());
		Mat out(rows,cols,img.type());

		float mvx=img.cols*(-horizontal)*(horizontal<0);
		float mvy=img.rows*(-vertical)*(vertical<0);
		float t[2][3];
		t[0][0]=1;
		t[0][1]=0;
		t[0][2]=mvx;//mvx;
		t[1][0]=0;
		t[1][1]=1;
		t[1][2]=mvy;//mvy;
		Mat tr(2,3,CV_32F,t);
		warpAffine(img,mid,tr,mid.size());


		float m[3][3];
		m[0][0]=1;
		m[0][1]=horizontal; // +r/l-
		m[0][2]=0;
		m[1][0]=vertical; // -up/down+
		m[1][1]=1;
		m[1][2]=0;
		m[2][0]=0;
		m[2][1]=0;
		m[2][2]=1;
		Mat mat(3,3,CV_32F,m);
		warpPerspective(mid,out,mat,out.size());

		//imshow(name,out);
		img=out;
	}

public:

	SourceImage(string file,int export_mode=COLOR_RGB): name(file)
	{
		img_color = imread(file,1);
		if(img_color.empty())
		{
			cout << "Cannot open file " << file << endl;
			exit(1);
		}
		img_gray = imread(file,0);
		img_var = imread(file,0);

		if(export_mode==COLOR_RGB)
		{
			img_out=&img_color;
		}
		else if(export_mode==GRAYSCALE)
		{
			img_out=&img_gray;
		}
		else if(export_mode==PROCESSED)
		{
			img_out=&img_var;
		}
		else
		{
			cout << "Runtime Error: unknown export mode" << endl;
			exit(-1);
		}
	}

	void lean(float vertical=0,float horizontal=0)
	{
		shear(img_var,vertical,horizontal);
		if(img_out!=&img_var)
		{
			shear(*img_out,vertical,horizontal);
		}
	}

	void applyFilters()
	{
		Size gaussSize;
		gaussSize.width = 5;
		gaussSize.height = 5;
		Mat element = getStructuringElement( MORPH_RECT, Size( 3, 3 ), Point( 2, 2 ) );

		GaussianBlur(img_var,img_var,gaussSize,0,0);
		dilate(img_var,img_var,element);
		Canny(img_var,img_var,160,80);
	}

	void findRectPositions()
	{
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		findContours(img_var, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
		boundRect.clear();
		boundRect.resize( contours.size() );
		vector<vector<Point> > contours_poly( contours.size() );

		for(unsigned int i = 0; i < contours.size() ; i++)
		{
			approxPolyDP( Mat(contours[i]), contours_poly[i], 100, true );
			boundRect[i] = boundingRect( Mat(contours_poly[i]) );
		}
	}

	void matchPlateProportions(float lb=3.84,float ub=6.87)
	{
		plates_rect.clear();
		for(unsigned int i = 0; i< boundRect.size(); i++ )
		{
			float proportions = ((float)boundRect[i].width/(float)boundRect[i].height);

			if( (proportions > lb) && (proportions < ub))
			{
				// drobne przesunięcie - poprawia OCR
				boundRect[i].x += 8;
				boundRect[i].y += 0;
				boundRect[i].width *= 0.95;
				boundRect[i].height *= 0.9;
				plates_rect.push_back(boundRect[i]);
				//cout << "i: " << i << " " << proportions << endl;
			}
		}
	}

	pair<float, float> quality(Mat img)
	{
		cvtColor(img,img,CV_BGR2HSV);
		vector<Mat> hsv;
		split(img,hsv);
		cvtColor(img,img,CV_HSV2BGR);

		int histSize = 256;
		float range[] = { 0, 256 } ;
		const float* histRange = { range };

		Mat saturation,value;
		calcHist( &hsv[1], 1, 0, Mat(), saturation, 1, &histSize, &histRange);
		calcHist( &hsv[2], 1, 0, Mat(), value, 1, &histSize, &histRange);

		float sat[256],val[256],sat_sum=0,val_sum=0;
		for(int i=0; i<256; i++)
		{
			sat[i]=saturation.at<float>(i,0);
			sat_sum+=sat[i];
			val[i]=value.at<float>(i,0);
			val_sum+=val[i];
		}
		for(int i=0; i<256; i++)
		{
			sat[i]=sat[i]/sat_sum;
			val[i]=val[i]/val_sum;
		}

		float qsat=0,savg=0,marg=ceil(256*0.2);
		for(int i=0; i<256; i++)
		{
			if(i<marg)
			{
				qsat+=sat[i];
			}
			savg+=i*sat[i];
		}
		savg=(256-savg)/256;
		qsat=sqrt(qsat*savg);


		float qval,qb=0,qw=0;
		float mid=256/2;
		for(int i=0; i<256; i++)
		{
			float wg=abs(mid-i);
			if(i<mid)
			{
				qb+=(wg*wg*val[i]);
			}
			else
			{
				qw+=(wg*wg*val[i]);
			}
		}
		qw=qw/(mid*mid);
		qb=qb/(mid*mid);
		qval=2*sqrt(qw*qb);

		pair<float, float> x;
		x.first = qsat;
		x.second = qval;
		return x;
	}


	void calculateQuality()
	{
		for(unsigned int i = 0; i < plates_img.size() ; i++)
		{
			sv.push_back(quality(plates_img[i]));
		}
	}

	void deletePoorPlates(float s,float v)
	{
		for(unsigned int i = 0; i < plates_img.size() ; i++)
		{
			if((sv[i].first <= s) && (sv[i].second <= v))
			{
				swap(sv[i],sv.back());
				sv.pop_back();

				swap(plates_img[i],plates_img.back());
				plates_img.pop_back();
				i--;
			}
		}

	}


	void drawPlates()
	{
		if(img_out!=&img_color)
		{
			//cvtColor(*img_out,*img_out,CV_GRAY2RGB,3);
		}
		RandomColor rc;
		for(unsigned int i = 0; i< plates_rect.size(); i++ )
		{
			rectangle( *img_out, plates_rect[i].tl(), plates_rect[i].br(), rc.getColor(), 2);
		}
	}

	void cropPlates()
	{
		plates_img.clear();
		for(unsigned int i = 0; i< plates_rect.size(); i++ )
		{
			plates_img.push_back((*img_out)(plates_rect[i]));
		}
	}


	vector<Mat> getPlates()
	{
		return plates_img;
	}

	vector<pair<float, float> >* getSv()
	{
		return &sv;
	}

	void displayPlates()
	{
		for(unsigned int i = 0; i< plates_img.size(); i++ )
		{
			imshow("plate",plates_img[i]);
			waitKey(0);
		}

	}

	void displayImage()
	{
		imshow(name,*img_out);
		waitKey(0);
	}
};
#endif
