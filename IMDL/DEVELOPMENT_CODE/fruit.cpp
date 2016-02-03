#include "stdafx.h"

#include "fruit.h"



fruit::fruit()
{
}


fruit::~fruit()
{
}

int fruit::GetXPos() {
	return xPos;
}

int fruit::GetYPos() {
	return yPos;
}

int fruit::GetArea() {
	return area;
}

Scalar fruit::GetHSVmin() {
	return HSVmin;
}

Scalar fruit::GetHSVmax() {
	return HSVmax;
}
//--	--	-	-	-	-	-	-	-	

void fruit::SetXPos(int x) {
	xPos = x;
}

void fruit::SetYPos(int y) {
	yPos = y;
}

void fruit::SetArea(int newArea) {
	area = newArea;
}

void fruit::SetHSVmin(Scalar newMin) {
	HSVmin = newMin;
}

void fruit::SetHSVmax(Scalar newMax) {
	HSVmax = newMax;
}