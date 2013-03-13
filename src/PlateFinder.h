#ifndef PLATEFINDER_H_INCLUDED
#define PLATEFINDER_H_INCLUDED


class PlateFinder
{
	SourceImage src,curr;
	float dev_curr,dev_step,dev_max;
	vector<string> results;

public:

	PlateFinder(SourceImage &in,float step=0.05f,float max=0.6f): src(in), curr(in)
	{
		dev_curr=0;
		dev_max=max;
		dev_step=step;
	}

	string OCR(Mat &in)
	{
		TessBaseAPI *OCR = new TessBaseAPI();

		OCR->Init(NULL,"eng");
		OCR->SetVariable("tessedit_char_whitelist", "0123456789|ABCDEFGHIJKLMNOPQRSTUVWXYZ ");
		OCR->SetImage((uchar*)in.data, in.size().width, in.size().height, in.channels(), in.step1());
		OCR->Recognize(NULL);

		string plate_numbers =  OCR->GetUTF8Text();

		OCR->Clear();
		OCR->End();

		return plate_numbers;
	}


	void ORC_plates(vector<Mat> tablice)
	{
		results.clear();
		for(unsigned int i=0 ; i<tablice.size() ; i++)
		{
			string wynik = this->OCR(tablice[i]);
			wynik.erase(std::remove(wynik.begin(), wynik.end(), '\n'), wynik.end());
			results.push_back(wynik);

		}

	}

	int evaluate(string x)
	{
		int spaces = 0;
		int numeric = 0;
		int alpha = 0;
		int result;

		if(!(x.size() <= 14) && (x.size() > 5))
			result -= 10;

		for(unsigned int i = 0; i < x.length() ; i++)
		{
			if( (i != 0) && (x[i] == ' ') ) spaces++;
			if((x[i] >= 65)&&(x[i] <= 90)) alpha++;
			if((x[i] >= 48)&&(x[i] <= 57)) numeric++;
		}
		result = 1;
		if(spaces > 3) result -= 1;
		if(alpha > 5 ) result -= 1;
		if(numeric > 5) result -= 1;
		if(spaces <= 2) result += 1;
		if(alpha <= 4) result += 1;
		if(numeric <= 4) result += 1;
		if(numeric+alpha < 5 ) result -= 3;
		return result;
	}

	string OCR_winner()
	{
		int winner_id = 0;
		int max_result = 0;
		int tmp_result = 0;

		for(unsigned int i = 0; i < results.size(); i++)
		{

			tmp_result = this->evaluate(results[i]);
			//cout << "-> " << results[i] <<  " :" << tmp_result << endl;

			if(tmp_result >= max_result)
			{
				max_result = tmp_result;
				winner_id = i;
				if(tmp_result > 1)
				{
					cout << "-> " << results[i] <<  " :" << tmp_result <<  endl;
				}

			}
		}

		if(results.size() > 0)
			return results[winner_id];
		else
			return "";

	}


	bool next()
	{
		if(dev_curr<=0)
		{
			dev_curr=dev_step-dev_curr;
			if(dev_curr>dev_max)
			{
				return false;
			}
		}
		else
		{
			dev_curr=-dev_curr;
		}
		curr= SourceImage(src);
		curr.lean(dev_curr);
		return true;
	}

	void execute()
	{
		//SourceImage img(argv[1],PROCESSED);
		//img.lean(-0.4);
		do
		{

//		std::cout << "dev=" << dev_curr << std::endl;
			curr.applyFilters();
			curr.findRectPositions();
			curr.matchPlateProportions();
			//curr.drawPlates();
			//curr.displayImage();
			curr.cropPlates();
			curr.calculateQuality();
			curr.deletePoorPlates(0.86,0.1);
			//curr.displayPlates();

			if(curr.getPlates().size() > 0 )
			{
				ORC_plates(curr.getPlates());
				OCR_winner();
			}

		}
		while(next());

	}

};


#endif // PLATEFINDER_H_INCLUDED
