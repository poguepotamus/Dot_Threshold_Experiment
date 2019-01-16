//////////////////////////////////////////////////////////////////////
//
// TwoAFCstaircase.h: implementation of the TwoAFC class
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>	//Header file for the windows library
#include <stdlib.h>		//Header filer for time
#include <time.h>		//Header declares time and date function e.g. clock_t and CLK_TCK
#include <stdio.h>		//Header file for C standard library (fopen, printf, etc.)
#include <dos.h>		//Header file for sleep fucntion
#include <math.h>		//Header file for math function e.g. cos, sin, etc.
#include <ctime>		//Header file for srand() and rand()
#include <cstdlib>		//Header file C Standard General Utilities Library
#include <fstream>		//Header file for output file stream
#include <iostream>		//Header file for standard iostream name forms
#include <algorithm>	//Header file for the random_shuffle() function
#include "TwoAFCstaircase.h" //Header file to include the TwoAFC class
#include "global.h"		//Header file to include external variables

using namespace std;	//Eliminates the need to type std::
char	datafilename2[100];				//Filename string
ofstream datafile2;		//Output class

void TwoAFC::StairOutput(void)
{
	bool filenameok2 = 0;	//Boolean flag
	
	strcpy_s(datafilename2, "data\\TE_");	//Change this to get the data in the right place
	strcat_s(datafilename2, num);					//Append initials
	strcat_s(datafilename2, "_");					//Append underscore
	strcat_s(datafilename2, init);					//Append subject number
	strcat_s(datafilename2, "_Dot_STAIR_3.8");					//Append underscore
	strcat_s(datafilename2, ".txt");				//Append file extension
	
	fstream fs(datafilename2, ios_base::in);		//Attempt open for read
	while(!filenameok2)
	{
		if (!fs)	//File doesn't exist; create a new one
		{
			fs.open(datafilename2, ios_base::out); 
			filenameok2 = 1;
		}
		else //Ok, file exists; close
		{
			fs.close();
			//Pop Up A Message Box Letting User Know The Program Is Closing.
			MessageBox(NULL,"File Already Exists.","ERROR",MB_OK|MB_ICONSTOP);
			exit(0);	
		}
		datafile2.open(datafilename2); 
		if (!datafile2)//data directory not found
		{   
			//Pop Up A Message Box Letting User Know The Program Is Closing.
			MessageBox(NULL,"Directory Not Found.","ERROR",MB_OK|MB_ICONSTOP);
			exit(0);
		}
	}
}

/* Initialize staircase */
void TwoAFC::initStaircase() {

	cout << "Staircase's Start Value: " << currentValue << endl << endl;	//Output the start contrast value at each trial
	datafile2 << "Staircase's Start Value: " << currentValue << endl << endl;	//Output the start contrast value at each trial
}

/* Function to count the number of reversals */
void TwoAFC::countReversals() {
	//If current direction and the last direction are opposite of each other then equal to one reversal
	if (lastDirection == 1 && direction == -1 || lastDirection == -1 && direction == 1) {
		reversalTrigger = 1;
		numbReversals++;		//Increase the number of reversals by 1
	}
	else {						//Otherwise, there is no reversal and the number of current reversals remains the same
		reversalTrigger = 0;
		numbReversals = numbReversals;
	}
}

/* Function to store the data of each reversal */
void TwoAFC::storeReversal() {
	reversalData[rN] = currentValue;						//Store the current contrast value in each reversal's array
	currentReversalData = currentValue;						//Keeps track of the current reversal value

	//cout << "Staircase's Reversal Data: " << reversalData[rN] << endl;	//Output current value store in the array of each reversal

	/* Sum all reversals */
	sum = sum + reversalData[rN];							//Sum up all the store reversal data
	average = sum/maxReversals;

	/* Sum the last six reversals */
	sum6 = reversalData[7] + reversalData[6] + reversalData[5] + reversalData[4] + reversalData[3] + reversalData[2];
	average6 = sum6/6;
}

/* Function to evaluate correct & incorrect response & count the number of correct responses */
void TwoAFC::evalCorrect() {
	if (correct == 1) {			//If response is correct
		cout << "Staircase's Response: Correct" << endl;	//Output 'correct'
		datafile2 << "Staircase's Response: Correct" << endl;	//Output 'correct'
		responseCorrect++;						//Add 1 to the number of correct response
		countCorrect++;							//Add 1 to the count of correct response
	}
	else if (correct == 0) {	//If response is incorrect
		cout << "Staircase's Response: Incorrect" << endl;	//Output 'incorrect'
		datafile2 << "Staircase's Response: Incorrect" << endl;	//Output 'incorrect'
		direction = 1;			//Change staircase direction to 'Up'
		countCorrect = 0;		//Reset the count of correct response to 0
		countReversals();		//Calls the function
	}
	else {									//Otherwise, specify that the response is null
		responseCorrect = responseCorrect;	//Set the the number of correct response to stay the same
		countCorrect = 0;					//Reset the count of correct response to 0
	}

	if (countCorrect == 2) {	//If the number of correct response reaches 2
		direction = -1;			//change staircase direction to 'Down'
		countCorrect = 0;		//Reset the count of correct response to 0
		countReversals();		//Calls the function
	}
	else {								//Otherwise
		countCorrect = countCorrect;	//Set the number of correct response to stay the same
	}
}

/* Function to keep track of staircase directions */
void TwoAFC::calcResponse() {
	if (direction == 1) {		//If staircase direction is 'Up'
		cout << "Staircase Up" << endl;		//Output 'up'
		datafile2 << "Staircase Up" << endl;		//Output 'up'
		lastDirection = 1;			//Store current staircase direction as last staircase direction
		if (currentValue < maxValue) {				//If current contrast value is less than the maximum contrast value
			currentValue = ceil(currentValue + (currentValue*percent));	//Then increase the current contrast value by 10%
			if (reversalTrigger == 1) {				//If there is 1 reversal
				cout << "Staircase's # Reversals: " << numbReversals << endl;		//Output the current number of reversals
				cout << "Staircase's Current Value: " << currentValue << endl;	//Output the current contrast value
				datafile2 << "Staircase's # Reversals: " << numbReversals << endl;		//Output the current number of reversals
				datafile2 << "Staircase's Current Value: " << currentValue << endl;	//Output the current contrast value
				storeReversal();				//Calls the function
				rN++;							//Increase the reversal number by 1
			}
		}
		else {				//Otherwise
			currentValue = currentValue;			//Set current value of contrast to stay the same
		}
		direction = 0;			//Reset staircase's direction to neutral
	}
	else if (direction == -1) {	//Else if staircase's direction is 'Down'
		cout << "Staircase Down" << endl;		//Output 'Down'
		datafile2 << "Staircase Down" << endl;		//Output 'Down'
		lastDirection = -1;			//Store current staircase direction as last staircase direction
		if (currentValue > minValue) {				//If current contrast value is greater than the maximum contrast value
			currentValue = ceil(currentValue - (currentValue*percent)); //Then decrease the current contrast value by 10%
			if (reversalTrigger == 1) {				//If there is 1 reversal
				cout << "Staircase's # Reversals: " << numbReversals << endl;		//Output the current number of reversals
				cout << "Staircase's Current Value: " << currentValue << endl;		//Output the current contrast value
				datafile2 << "Staircase's # Reversals: " << numbReversals << endl;		//Output the current number of reversals
				datafile2 << "Staircase's Current Value: " << currentValue << endl;		//Output the current contrast value
				storeReversal();				//Calls the function
				rN++;							//Increase the reversal number by 1
			}
		}
		else {								//Otherwise
			currentValue = currentValue;	//Set current value of contrast to stay the same
		}
		direction = 0;						//Reset staircase's direction to neutral
	}
}

//void TwoAFC::finalCalc() {
//	cout << endl << "Staircase's # of correct: " << responseCorrect << endl;	//Output the total number of correct response
//	cout << "Staircase's Total trials: " << totalTrials << endl;				//Output the total number of trials
//	totalCorrect = (responseCorrect*100.0)/totalTrials;				//Calculates the the number of correct response in percentage
//	cout << "Staircase's % of correct: " << totalCorrect << endl;				//Output the percentage of correct response
//	cout << "Staircase's Current Value: " << currentValue << endl;				//Output the current value of contrast
//	cout << "Staircase's All Reversal Average: " << average << endl;					//Output the average value of contrast
//	cout << "Staircase's Last 8 Reversals Average: " << average8 << endl << endl;		//Output the average value of contrast
//}