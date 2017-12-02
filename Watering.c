//Group Member: Veronica, Kelly, Max
//Group Number: Three
//Name: Hardy Harvesters


#include <stdio.h>
#include <ugpio/ugpio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <oled-exp.h>


int temperature(int value, int cycle_counter);
int moisture(int value, int cycle_counter);
void update(int value_t, int value_m);
void max (const int dataset[], const int data[],const int size);
void min (const int dataset[],const int data[],const int size);
void average (const int dataset[], const int data[], const int size);

int perfectSoil = 0;
int not_perfect_soil = 0;
int bad = 0;
int optimal = 0;
int not_optimal = 0;
int update_t = 0; //0 = happy, 1 = ok, 2 = bad
int update_m = 0;
int c = 0;
int cycle_counter = 1;


int main() {

	int rq, rv;

	int light1 = gpio_request(1, NULL);
	int light2 = gpio_request(2, NULL);
	int light3 = gpio_request(3, NULL);

	if (light1 != 0 || light2 != 0 || light3 != 0) {
		printf("ERROR: Unable to turn on lights");
		char logPath[] = "/root/logFiles/log.txt";
		FILE *log = fopen(logPath, "a");
		if (log == NULL) {
			printf("Error opening log file");
		} else {
			time_t rawtime;
			struct tm * timeinfo;

			time (&rawtime);
			timeinfo = localtime(&rawtime);
			fprintf(log, "ERROR: Unable to turn on lights at %s\n", asctime(timeinfo));
			fclose(log);
		}
	}

	gpio_direction_output (2, 1); //green

	gpio_direction_output(3, 1); //yellow

	gpio_direction_output(1, 1); //red

	sleep(10);

	gpio_direction_output (2, 0); //green

	gpio_direction_output(3, 0); //yellow

	gpio_direction_output(1, 0); //red


	char sensorpath[] = "/dev/ttyS1";

	FILE *sensor;

	sensor = fopen(sensorpath, "r");
	if (sensor == NULL) {
		printf("Error opening file");

		char logPath[] = "/root/logFiles/log.txt";
		FILE *log = fopen(logPath, "a");
		if (log == NULL) {
			printf("Error opening log file");
		} else {
			time_t rawtime;
			struct tm * timeinfo;

			time (&rawtime);
			timeinfo = localtime(&rawtime);
			fprintf(log, "FATAL: Unable to open file at %s\n", asctime(timeinfo));
			fclose(log);
		}

		exit(1);
	}

	int oledStatus = oledDriverInit ();
	if (oledStatus != 0) {
		printf("ERROR: Unable to open OLED driver");

		char logPath[] = "/root/logFiles/log.txt";
		FILE *log = fopen(logPath, "a");
		if (log == NULL) {
			printf("Error opening log file");
		} else {
			time_t rawtime;
			struct tm * timeinfo;

			time (&rawtime);
			timeinfo = localtime(&rawtime);
			fprintf(log, "ERROR: Unable to open OLED driver at %s\n", asctime(timeinfo));
			fclose(log);
		}
	}

	oledSetDisplayPower (1);

	int image;
	char imagefile[] = "/root/image.lcd";
	uint8_t *buffer = (uint8_t *) malloc(OLED_EXP_WIDTH*OLED_EXP_HEIGHT/8 * sizeof *buffer); //allocate memory for the buffer

	image = oledReadLcdFile(imagefile, buffer); //param?

	//displays image on OLED
	if (image == EXIT_SUCCESS){
		image = oledDraw (buffer, OLED_EXP_WIDTH*OLED_EXP_HEIGHT/8);
	}

	sleep(10);
	int i = 0;
	char previous = 0;


	for (int j = 0; j < 40; j++) {

		char numberChar[10];
		char moisutre_or_temp = getc(sensor);

		if (moisutre_or_temp == 'm' || moisutre_or_temp == 't') {
			fgets(numberChar, 10, sensor);
			c = atof(numberChar);
		}


		// int oledClear (); //clears the screen after every interval
		int stats_m[5];
		int stats_t[5];

		if (moisutre_or_temp == 'm' && c != 0) {

			moisture(c, cycle_counter);
			update_m = moisture (c, cycle_counter);
			stats_m[i] = c;

			if (moisutre_or_temp == previous) {
				char logPath[] = "/root/logFiles/log.txt";
				FILE *log = fopen(logPath, "a");
				if (log == NULL) {
					printf("Error opening log file");
				} else {
					time_t rawtime;
					struct tm * timeinfo;

					time (&rawtime);
					timeinfo = localtime(&rawtime);
					fprintf(log, "WARNING: Missing temperature value at %s\n", asctime(timeinfo));
					fclose(log);
				}
			}

			previous = 'm';

		}

			//moisutre_or_temp is 1 indicating that it is now a temperaatue value
		else if (moisutre_or_temp == 't' && c != 0){



			temperature(c, cycle_counter); //calls function
			update_t = temperature (c, cycle_counter);
			stats_t[i] = c;

			i++;
			cycle_counter++;
			update(update_t, update_m);
			if (cycle_counter == 6){


				oledClear();
				max(stats_m, stats_t, i);
				min(stats_m, stats_t, i);
				average (stats_m, stats_t, i);
				i = 0;

			}
			if (cycle_counter == 7){

				cycle_counter = 0;
			}

			if (moisutre_or_temp == previous) {
				char logPath[] = "/root/logFiles/log.txt";
				FILE *log = fopen(logPath, "a");
				if (log == NULL) {
					printf("Error opening log file");
				} else {
					time_t rawtime;
					struct tm * timeinfo;

					time (&rawtime);
					timeinfo = localtime(&rawtime);
					fprintf(log, "WARNING: Missing moisture value at %s\n", asctime(timeinfo));
					fclose(log);
				}
			}

			previous = 't';


		}



		printf ("\n"); //seperates each interval by a line

	}

	gpio_direction_output (2, 0); //green
	gpio_direction_output (1, 0);
	gpio_direction_output (3, 0);
	oledSetDisplayPower(0);


	if (!rq) {
		printf("> unexporting gpio\n");
		if (gpio_free(1) < 0)
		{
			perror("gpio_free");
		}
	}
	if (!rq) {
		printf("> unexporting gpio\n");
		if (gpio_free(2) < 0)
		{
			perror("gpio_free");
		}
	}
	if (!rq) {
		printf("> unexporting gpio\n");
		if (gpio_free(3) < 0)
		{
			perror("gpio_free");
		}
	}


	fclose(sensor);

}

void max (const int dataset[], const int data[], const int size){

	int i = 0;
	int largest_value_m = dataset[0];
	int largest_value_t = data[0];
	char moisture[30];
	char temperature[30];
	char degrees = '%';
	char tempe = '*';
	char cel = 'C';
	char cycle[] = "Two Minute Update";

	while (i < size){

		int value = dataset[i];
		int number = data[i];

		if (value > largest_value_m){

			largest_value_m = value;
		}
		if (number > largest_value_t){

			largest_value_t = number;
		}

		i = i + 1;
	}

	oledSetCursor(0,3);
	oledWrite (cycle);
	oledSetCursor(1,0);
	snprintf (moisture, sizeof(moisture), "Max moisture: %d%c\n", largest_value_m, degrees);
	oledWrite(moisture);
	oledSetCursor(2,0);
	snprintf (temperature, sizeof(temperature), "Max temp: %d%c%c\n", largest_value_t, tempe,cel);
	oledWrite(temperature);

}
void min (const int dataset[], const int data[], const int size){

	int i = 0;
	int smallest_value_m = dataset[0];
	int smallest_value_t = data[0];
	char moisture[30];
	char temperature[30];
	char degrees = '%';
	char tempe = '*';
	char cel = 'C';

	while (i < size){

		int value = dataset[i];
		int number = data[i];

		if (value < smallest_value_m){

			smallest_value_m = value;
		}
		if (number < smallest_value_t){

			smallest_value_t = number;
		}

		i = i + 1;
	}

	oledSetCursor(3,0);
	snprintf (moisture, sizeof(moisture), "Min moisture: %d%c\n", smallest_value_m, degrees);
	oledWrite(moisture);
	oledSetCursor(4,0);
	snprintf (temperature, sizeof(temperature), "Min temp: %d%c%c\n", smallest_value_t, tempe, cel);
	oledWrite(temperature);

}


void average (const int dataset[], const int data[], const int size){

	int i = 0;
	int average_m = 0;
	int average_t = 0;
	char moisture[30];
	char temperature[30];
	char degrees = '%';
	char tempe = '*';
	char cel = 'C';

	while (i < size){

		int value = dataset[i];
		int number = data[i];
		average_m = average_m + value;
		average_t = average_t + number;

		i = i + 1;
	}

	average_m = average_m/(size);
	average_t = average_t/(size);

	oledSetCursor(5,0);
	snprintf (moisture, sizeof(moisture), "Avg moisture: %d%c\n", average_m, degrees);
	oledWrite(moisture);
	oledSetCursor(6,0);
	snprintf (temperature, sizeof(temperature), "Avg temp: %d%c%c\n", average_t, tempe, cel);
	oledWrite(temperature);
}

int moisture(int value, int cycle_counter){

	oledClear();

	printf("Moisture Value: %d", value);
	printf(".");

	char logPath[] = "/root/logFiles/log.txt";
	FILE *log = fopen(logPath, "a");
	if (log == NULL) {
		printf("Error opening log file");
	} else {
		time_t rawtime;
		struct tm * timeinfo;

		time (&rawtime);
		timeinfo = localtime(&rawtime);
		fprintf(log, "INFO: Moisture measurement at %s: %d\n", asctime(timeinfo), value);
		fclose(log);
	}


	char moisture[30];
	char degrees = '%';
	oledSetCursor(0,0);
	snprintf (moisture, sizeof(moisture), "The moisture is %d%c\n", value, degrees);
	oledWrite(moisture);
	int state = 0;

	if (value <= 0){
		//super dry
		printf (" Plant needs watering now!\n");
		state = 2;
		not_perfect_soil++;


	}
	else if (value > 0 && value <= 40){
		//more water needed
		printf (" Plant needs more water for optimal conditions.\n");
		state = 1;
		not_perfect_soil++;

	}
	else if (value >=40 && value<45){
		//a little bit dry
		printf (" Plant is a little dry. Please add a bit more water for optimal condiions.\n");
		state = 1;
		not_perfect_soil++;

	}
	else if (value >= 45 && value<= 55){
		// optimal moisture level
		perfectSoil+=1;
		printf (" Plant's soil is perfect!\n");
		state = 0;
		perfectSoil++;

	}
	else if (value >55 && value <=60){
		//a little bit too much water
		printf (" CAUTION: A little too much water in here.\n");
		state = 1;
		not_perfect_soil++;

	}
	else if (value > 60){
		//way too much water
		printf (" EMERGENCY: There is too much water in here and it is not healthy for the plant.\n");
		state = 2;
		not_perfect_soil++;
	}

	if (cycle_counter == 6){

		if (not_perfect_soil >= perfectSoil){

			printf ("In the last two minutes your plant has only had the right moisture for %d", perfectSoil);
			printf (" cycles. Please pay attention to the moisture level!\n");

		}
		else {

			printf ("In the last two minutes your plant had the right moisture for %d", perfectSoil);

		}

		perfectSoil = 0;
		not_perfect_soil = 0;

	}

	return state;



}


int temperature(int value, int cycle_counter){


	char temperature[30];
	char degrees[] = "degrees";
	oledSetCursor(2,0);
	snprintf (temperature, sizeof(temperature), "The temperature is %d\n", value);
	oledWrite(temperature);
	oledSetCursor(3,0);
	oledWrite(degrees);
	int state = 0;

	char logPath[] = "/root/logFiles/log.txt";
	FILE *log = fopen(logPath, "a");
	if (log == NULL) {
		printf("Error opening log file");
	} else {
		time_t rawtime;
		struct tm * timeinfo;

		time (&rawtime);
		timeinfo = localtime(&rawtime);
		fprintf(log, "INFO: Temperature measurement at %s: %d\n", asctime(timeinfo), value);
		fclose(log);
	}

	if(value <= 0){
		//really cold
		printf ("The temperature is currently: %d", value);
		printf(" degrees celsius. EMERGENCY: Plant needs warmer living conditions immediately!\n");

		state = 2;
		bad++;

	}
	else if ( value > 0 && value <= 10){
		//still not suitable living conditions
		printf ("The temperature is currently: %d", value);
		printf(" degrees celsius. Please find warmer condiions immediately\n");

		state = 2;
		bad++;

	}
	else if (value > 10 && value < 20) {
		//not optimal living conditions
		printf ("The temperature is currently: %d", value);
		printf(" degrees celsius. Please move the plant to warmer conditions for optimal performance\n");

		state = 1;
		not_optimal++;
	}
	else if (value >= 20 && value < 30){
		//optimal living conditions
		printf ("The temperature is currently: %d", value);
		printf(" degrees celsius. The plant is happy\n");

		state = 0;
		optimal++;

	}
	else if (value >= 30 && value <= 40){
		//getting pretty warm
		printf ("The temperature is currently: %d", value);
		printf(" degrees celsius. The temperature is hot! Take caution.\n");

		state = 1;
		not_optimal++;
	}
	else if (value > 40){
		//really hot
		printf ("The temperature is currently: %d", value);
		printf(" degrees celsius. EMERGENCY: Plant needs cooler living conditions immediately!\n");

		state = 2;
		bad++;

	}


	if (cycle_counter == 6){



		if (optimal > bad){

			if (optimal > not_optimal){

				printf ("In the last two minutes your plant has been happy for %d", optimal);
				printf (" cycles. KEEP UP THE GOOD WORK!");
				printf("\n");
			}
			else if (not_optimal >= optimal){

				printf ("In the last two minutes your plant hasn't been living in optimal conditions. Please take better care of your plant for optimal health.");
				printf ("\n");

			}

		}

		else if (optimal < bad){

			if (bad > not_optimal) {

				printf ("In the last two minutes your plant has been living in an unsuitable environemtn for %d", bad);
				printf (" cycles. PLEASE TAKE CARE OF THE PLANT!");
				printf ("\n");
			}
			else if (not_optimal >= bad){

				printf ("In the last two minutes your plant hasn't been living in optimal conditions. Please take better care of your plant for optimal health.");
				printf ("\n");
			}

		}

		else {

			printf ("In the last two minutes the temperature has been changing a lot. This change is not healthy for the plant. Please take caution!");
			printf ("\n");

		}

		optimal = 0;
		bad = 0;
		not_optimal = 0;
	}

	return state;


}

void update(int value_t, int value_m){


	oledSetCursor(5,0);

	if ( value_t == 0 && value_m == 0){

		char update[] = "Plant is HAPPY!";
		oledWrite(update);

		gpio_direction_output (2, 1); //green
		gpio_direction_output (1, 0); //red
		gpio_direction_output (3, 0); //yellow

	}
	else if (value_t == 1 && value_m == 1){

		char update[] = "Pay attention to the";
		oledWrite (update);
		oledSetCursor (6,0);
		char update_2[] = "temp and moisture";
		oledWrite (update_2);

		gpio_direction_output (2, 0); //green
		gpio_direction_output (1, 0); //red
		gpio_direction_output (3, 1); //yellow

	}
	else if (value_t == 2 && value_m == 2){

		char update[] = "EMERGENCY: Plant";
		oledWrite(update);
		oledSetCursor(6,0);
		char update_2[] = "is NOT satisfied";
		oledWrite(update_2);

		gpio_direction_output (2, 0); //green
		gpio_direction_output (1, 1); //red
		gpio_direction_output (3, 0); //yellow

	}
	else if (value_t == 0 && value_m != 0){

		char update[] = "Temp is fine";
		oledWrite (update);
		oledSetCursor(6,0);
		char update_2[] = "Moisture is not";
		oledWrite (update_2);

		gpio_direction_output (2, 0); //green
		gpio_direction_output (1, 0); //red
		gpio_direction_output (3, 1); //yellow
	}
	else if (value_t != 0 && value_m == 0){

		char update[] = "Moisture is fine";
		oledWrite (update);
		oledSetCursor(6,0);
		char update_2[] = "Temerature is not";
		oledWrite(update_2);

		gpio_direction_output (2, 0); //green
		gpio_direction_output (1, 0); //red
		gpio_direction_output (3, 1); //yellow
	}
	else {

		char update[] = "Neither temp or";
		oledWrite (update);
		oledSetCursor (6,0);
		char update_2[] = "moisture is optimal";
		oledWrite(update_2);

		gpio_direction_output (2, 0); //green
		gpio_direction_output (1, 0); //red
		gpio_direction_output (3, 1); //yellow

	}

}
