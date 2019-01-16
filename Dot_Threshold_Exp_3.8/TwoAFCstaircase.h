//////////////////////////////////////////////////////////////////////
//
// TwoAFCstaircase.h: interface for the TwoAFC class
//
//////////////////////////////////////////////////////////////////////

#pragma once

class TwoAFC {

private:
	int countCorrect;				//Count the number of correct response
	int direction;					//1 = up & -1 = down
	int lastDirection;				//Keep track of previous direction
	int reversalTrigger;			//1 = reversal, 0 = no reversal
	int rN;							//Reversal n (keeps track of reversal data)
	double sum;						//Sum all the data of ten reversals
	double sum6;					//Sum all the data of the last eight reversals
	double reversalData[300];		//Array to store the current contrast value of each reversal
	//double maxValue;				//Maximum value of contrast
	//double minValue;				//Minimum value of contrast
	double percent;					//Percentage of a step size
	
public:
	int correct;					//Determine if response is correct--1 = correct & 0 = incorrect
	int numbReversals;				//Number of reversals
	int maxReversals;				//Max number of reversals
	int totalTrials;				//Total number of trials
	double responseCorrect;			//Determine the number of correct response
	double totalCorrect;			//The total percentage of correct responses
	double currentReversalData;		//Current value of each reversal
	double currentValue;			//Current value of contrast
	double average;					//Sum divided by the # of max reversals
	double average6;				//Sum8 divided by the # of max reversals for the last eight reversals
	double maxValue;				//Maximum value of contrast
	double minValue;				//Minimum value of contrast

	TwoAFC() {			//Constructor	
		responseCorrect = 0.0;
		countCorrect = 0;
		direction = 0;
		lastDirection = 0;
		numbReversals = 0;
		rN = 0;	
		maxReversals = 8;
		sum = 0;
		sum6 = 0;
		maxValue = 325;
		minValue = 5;
		percent = 0.20;						//Percent to multiply the currentValue
		currentValue = 160;					//Current value of contrast/starting value of contrast
		currentReversalData = 0;			//Current reversal data value
	}
	~TwoAFC() {}		//Destructor
	
	/* Member Functions (Methods) */ 
	void initStaircase();
	void countReversals();
	void storeReversal();
	void evalCorrect();
	void calcResponse();
	void finalCalc();
	void StairOutput();
};