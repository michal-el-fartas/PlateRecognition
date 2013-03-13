#ifndef RANDOMCOLOR_H
#define RANDOMCOLOR_H


class RandomColor
{
	int getByte(int lb,int ub);
public:
	RandomColor();
	Scalar getColor();
};


//#include "../include/RandomColor.h"

int RandomColor::getByte(int lb=20,int ub=230)
{
	return (rand()%(ub-lb))+lb;
}

RandomColor::RandomColor()
{
	srand(time(0));
}

Scalar RandomColor::getColor()
{
	return Scalar(getByte(),getByte(),getByte());
}


#endif // RANDOMCOLOR_H
