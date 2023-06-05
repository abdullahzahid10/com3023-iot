

 

//Neccessary libraries imported

#include "contiki.h" //Neccessary library for Contiki operations

#include <stdlib.h> //Allocation of memory for pointers and allocation

#include "dev/light-sensor.h" //Reads light

#include "dev/sht11-sensor.h" //Reads Temperature

#include <stdio.h> /* For printf() */

#include <math.h> //Math functions

 

//Defining Buffer

#define bufferCapacity 12 // Setting the capacity of my buffer to 12

#define A12into1 1 //Used for 12 into 1 aggregation where 12 outputs from buffer will be put in one

#define A4into1 3

#define mediumActivity 50 // Indicator that if my calculated standard deviation is above 50, then there should me medium type aggregation.

#define highActivity 150 // Indicator that if my calculated standard deviation is above 150, then there should me High aggregation.

#define NAFVAR 0.745 // Variance for Normalized autocorrelation function

#define SF 0.5 // Smoothing factor advanced 

// Seperator, used to seperate elements within array, so I dont repeat myself with ", "

static char seperator[] = ", ";

 

//Merging number into 2 parts, one part before the dp(Int) and one after.

//Part 1

int d1(float f) // Integer part

{

// Returns the float as an integer

return((int)f);

}

//Part 2

unsigned int d2(float f) // Fractional part

{

if (f>0)

return(100*(f-d1(f)));

else

return(100*(d1(f)-f));

}

 

// Temp and Light reading methods

// Get temperature readings

float getTemperature(void)

{

// For simulation sky mote

int tempADC = sht11_sensor.value(SHT11_SENSOR_TEMP_SKYSIM);

float temp = 0.04*tempADC-39.6; // skymote uses 12-bit ADC, or 0.04 resolution

// For xm1000 mote

//int tempADC = sht11_sensor.value(SHT11_SENSOR_TEMP);

//float temp = 0.01*tempADC-39.6; // xm1000 uses 14-bit ADC, or 0.01 resolution

return temp;

}

//Get light readings

float getLight(void)

{

float V_sensor = 1.5 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC)/4096;

// ^ ADC-12 uses 1.5V_REF

float I = V_sensor/100000; // xm1000 uses 100kohm resistor

float light_lx = 0.625*1e6*I*1000; // convert from current to light intensity

return light_lx;

}

 

//Getting the average

float average(float *stat, int arrSize){

// Setting initial sum to 0

float totalSum = 0.0;

// Setting an initialiser

int i = 0;

// For loop which will add all the numbers in the array until array size

for(i = 0; i < arrSize; i++)

// Adding all the numbers in array, += iteratively adds whilst the loop is running

totalSum += stat[i];

// Return call to be called when the method is called in later stages

return totalSum / arrSize;

}

 

//Getting Square Root, Babylon method concept from L3 applied.

float Square_Root(float r){

//Error tolerance

float errorTolerance = 0.01;

// Storing parameter argument into a float, 'S'

float S = r;

float i = 1.00;

// While loop to calculate Square root to get final answer, abstracted from Babylonian method.

while (S - i > errorTolerance){

S = 0.5 * (S + i);

i = r / S;

}

// Return the square root when called

return S;

}

 

//Log Standard Deviation to Cooja Sim.

void standardDeviationDisplay(float standardDeviation){

// Printing standard deviation with integer(before dp) and float point(after dp)

printf("\nStdDev : %d.%02u\n", d1(standardDeviation), d2(standardDeviation));

}

 

// Display aggregation type.

void aggregationDisplay(int aggChoice){

// Initial string which goes before aggregation type.

char start[] = "Aggregation = ";

// If statement, that will display either 12, 4 or 1 elements as the aggregation output.

if (aggChoice == bufferCapacity){

printf("%sNo aggregation is used\n", start);

} else if (aggChoice == A4into1)

{

printf("%s4-into-1\n", start);

} else if (aggChoice == A12into1)

{

printf("%s12-into-1\n", start);

}

}

 

// Display aggregation depending on which type of aggregation to be used. A Method which keeps everthing together

void aggregationTypeDisplay(float *aggregation, int aggChoice){

char starter[] = "X";

displayArrForAgg(aggregation, aggChoice, starter);

}

 

//Calculate Standard Deviation

float standardDeviationCalculation(float *buffer){

//Obtain the average of buffer

float averageOfBuffer = average(buffer, bufferCapacity),

//Sum before variance - Initialiser

totalSum = 0.0,

//The Variance - Initialiser

totalVar = 0.0;

// Initialising K to be used in for loop

int k;

// For loop which takes total sum

for (k = 0; k < bufferCapacity; k++){

// totalSum adding iteratively the square value of the differences.

totalSum += (buffer[k] - averageOfBuffer)*(buffer[k] - averageOfBuffer);

}

// totalVar will take the variance. ie. totalsum divided by buffer capacity 12.

totalVar = totalSum / bufferCapacity;

// Return to be called in main

return Square_Root(totalVar);

}

 

//Display Buffer contents

void displayBuffer(float *buffer){

//Start of Console log

printf("B = [");

int l;

//Loop over Buffer contents

for (l = 0; l < bufferCapacity; l++){

printf("%d", d1(buffer[l]));

// Nested if statement to seperate elements

if (l < bufferCapacity - 1){

printf("%s", seperator);

}

}

//End of console log

printf("]\n");

}

 

// Setting the type of aggregation to be used dependant on SD. Takes argument of SD and if loop is determined based of SD Value generated from Cooja.

int aggregationSetter(float standardDeviation){

int aggChoice = 0;

// If statement which will acknowledge SD value and make a decision on which aggregation type to go for.

if(standardDeviation < A4into1){

aggChoice = A12into1;

} else if (standardDeviation < highActivity){

aggChoice = A4into1;

} else {

aggChoice = bufferCapacity;

}

// Return the aggregation choice.

return aggChoice;

}

 

// Method to display array for aggregation

void displayArrForAgg(float *stat, int sizeOfArr, char type[]){

// Start of array

printf("%s = [", type);

// Initialiser a

int a;

// For loop to print aray, with nested loop to seperate elements

for (a = 0; a < sizeOfArr; a++){

printf("%d.%02u", d1(stat[a]), d2(stat[a]));

if (a < sizeOfArr - 1){

printf("%s", seperator);

}

}

// End of array.

printf("]\n\n");

}

 

// aggregateProcess method implied which takes iterative means on the go, reducing repeatability.

float *aggregateProcess(float *stat, float *AP, int aggChoice){

// Aggregation point with aggregationPoints means to be calculated

int aggregationPoint = bufferCapacity / aggChoice, m, n;

// For loop to get the readings and calcuulate arithmetic mean on the go

for (m = 0; m < aggChoice; m++){

// Initialize totalSum

float totalSum = 0.0;

// For loop for squareroot

for (n = m * aggregationPoint; n < (m + 1) * aggregationPoint; n++){

// Store data for all elements which are run as 'n' in totalSum

totalSum += stat[n];

}

// Get final answer by dividing totalSum with the aggregation Points iteratively.

AP[m] = totalSum / aggregationPoint;

}

// Return the sum

return AP;

}

 

// Display advanced feature line

void displayLine(){

printf("\nAdvanced Features\n");

}

 

float *autoCorrelation(float *buffer, float *NAF){

// Initialise P

int p;

 

// For loop to run over the 12 elements, k = 12;

for (p = 0; p < bufferCapacity; p++){

if (p == 0){

NAF[p] = buffer[p];

continue;

}

//  Elements shifted by 1 and compared to the original elements

NAF[p] = (NAFVAR * buffer[p]) + ((1 - NAFVAR) * NAF[p - 1]);

}

return NAF;

}

 

// Display findings.

void displayNAF(float *NAF){

displayArrForAgg(NAF, bufferCapacity, "Normalized autocorrelation Fuction:");

}


void dispSmoothingFac(){
 printf("\n\n\n\Smoothing factor = %d.%02u\n", d1(SF), d2(SF));
}

float *EMACalculation(float *buffer, float *EMA){
 int q;


	for(q=0; q<bufferCapacity; q++){


 int ybi = SF * buffer[q];
 int eqP2 = (SF - 1) * SF;
EMA[q] = ybi + eqP2;
}

return EMA;
}

void displayEMA(float *EMA){

displayArrForAgg(EMA, bufferCapacity, "EMA");

}

void SAXDisplay(){

printf("\SAX\n");

}

 

// Main method started from here.

/*---------------------------------------------------------------------------*/

PROCESS(sensor_reading_process, "Sensor reading process");

AUTOSTART_PROCESSES(&sensor_reading_process);

/*---------------------------------------------------------------------------*/

// Start of a thread, which will run concurrently.

PROCESS_THREAD(sensor_reading_process, ev, data)

{

// Declaring required variables.

static struct etimer timer;

PROCESS_BEGIN();

etimer_set(&timer, CLOCK_CONF_SECOND / 2); // 2 second duration

// Activate light and temp sensors/

SENSORS_ACTIVATE(light_sensor);

SENSORS_ACTIVATE(sht11_sensor);

//static float light_lx = getLight();

// Set count, measure, buffer, aggChoice, standardDeviation, aggregation, light, averagePrinter

// Start the tally to get one reading after the other

static int count = 0;

// Take in reading

static float measure;

// Buffer readings stored

static float *buffer;

// Choice of aggregation passed on as args

static int aggChoice;

// Standard deviation, currently set to 0 and will change later to actual standard deviation

static float standardDeviation = 0.0;

// Storing the aaggregation

static float *aggregation = 0;

// Variable storing the average

static float averagePrinter;

// Allocating memory for the buffer variable

buffer = (float *)malloc(bufferCapacity * sizeof(float));

static float *NAF;

static float *EMA;


 

// While loop to start the code.

while(1)

{

// Start of timer

PROCESS_WAIT_EVENT_UNTIL(ev=PROCESS_EVENT_TIMER);

// Get light readings and print them one by one, iteratively.

measure = d1(getLight() * 1);

printf("Reading = %d\n", d1(measure));

buffer[count] = measure;

count++;

// When count reaches the capacity of buffer, ie 12, it will display all neccessary information.

if(count == bufferCapacity){

// Store value of standard deviatiom

standardDeviation = standardDeviationCalculation(buffer);

// Set choice of aggregation

aggChoice = aggregationSetter(standardDeviation);

// Aggregate value for aggregation sets

aggregation = (float *)malloc(aggChoice * sizeof(float));

aggregation = aggregateProcess(buffer, aggregation, aggChoice);

// Print standard deviation

standardDeviationDisplay(standardDeviation);

// Print the buffer from the taken readings

displayBuffer(buffer);

// Print choice of aggregation, dependant on the choice of aggregation

aggregationDisplay(aggChoice);

// Print the aggregation type with choice of aggregation.

aggregationTypeDisplay(aggregation, aggChoice);

// Display Advanced Tasks

displayLine();

// Allocate memory for normalized autocorrelation variable

NAF = (float *)malloc(bufferCapacity * sizeof(float));

NAF = autoCorrelation(buffer, NAF);

// Display NAF

displayNAF(NAF);

dispSmoothingFac();

EMA = (float *)malloc(bufferCapacity * sizeof(float));

EMA = EMACalculation(buffer, EMA);

displayEMA(EMA);

SAXDisplay();

// Count set to 0, to restart the process for next set of readings.

count = 0;

}

// End of 2 second timer

etimer_reset(&timer);

}

// End of process

PROCESS_END();

}

/*---------------------------------------------------------------------------*/