#include "musicbox.h"
/**********************************************
© Alex Holtzman 2018

Betterbox - A simple Csound based, algorithmic 
music generator that runs from the command line

Made by Alex Holtzman, May 2018

Compile with: gcc betterbox_main.c -o betterbox
***********************************************/

int main(int argc, char **argv){
	boxdata data;

	//initialize our data structure
	init(&data);

	//get file info...
	getarguments(&data);

	//process our image file's data
	imageDataProcess(&data);
	
	//open our csd file that we'll be writing to.
	data.scorefile = fopen(data.filename, "w");

	//build our csd file's orchestra
	csdbuild(&data);

	//build our score
	scorebuild(&data);

	//close our csd file
	fclose(data.scorefile);

	//run our csound file from the command line
	runscore(&data);

    //clean up after ourselves
    //cleanup(&data);
	
	return 0;
}
