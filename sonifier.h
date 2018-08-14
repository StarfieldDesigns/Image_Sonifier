#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
//***********************************
//data structure for global variables
//***********************************
typedef struct {
	int nextnote[256];    //random note number
	float nextpan[256];   //random panning value
	float notelength;     //note length
	int testnote;         //used to remove duplicate note numbers
	int midi_note;        //root note of the piece
	int result;           //used to share the randomly generated note numbers with other instruments in the cscore
	int keychoice;        //the choice of mode for the piece
	float totalLength;    //total length of the score
	int printflag;        //used for error handling
	double length;           //number of random note generations to be made
    int numchanges;       //number of changes that will occur in the score
    int scorenum;         //indicates which csound orchestra was picked
    int blockFlag;		  //keeps track of when the score should process the new block's image data

	char filename[100];   //filename
	char commands[100];   //char array used to execute terminal commands (temporary)

    int channel;          //used to keep track of how many times blockAverages() runs
    int redAvg[500];      //a variable to store our red pixel-block averages
    int greenAvg[500];    //a variable to store our green pixel-block averages
    int blueAvg[500];     //a variable to store our blue pixel-block averages
    unsigned int blockSize;        //a variable to store the number of pixels in each block that the image is divided into
    int numBlocks;        //a variable to store the number of pixel blocks the image is broken into
    int brightness;
    int mainColor;
    int blockColor[100];   	  //some space to store the color average of every block of the image
    int blockBrghtnss[100];	  //some space to store the brightness average of every block of the image
    unsigned int ravgs;
    unsigned int rsum;
    unsigned int gavgs;
    unsigned int gsum;
    unsigned int bavgs;
    unsigned int bsum;
    int saveFlag;

	FILE *scorefile;      //our scorefile for csound to use
    FILE *red;            //file for our red pixel values
    FILE *green;          //file for our green pixel values
    FILE *blue;           //file for our blue pixel values
} boxdata;
// **********
// prototypes
// **********
// ----------
//csound functions:
int init(boxdata *a);
int getarguments(boxdata *data);
void csdbuild(boxdata *data);
float randnum();
int scales(boxdata *data);
void scorebuild(boxdata *data);
int runscore(boxdata *b);
int cleanup(boxdata *c);
//----------------------------
//image sonification functions:
int setBlockSize(int color);
int blockAverages(FILE* color, boxdata *data);
int imageDataProcess(boxdata *data);
int colorFinder(boxdata *data, int tflag);
// *********
// functions
// *********
int init(boxdata *a){

	a = malloc(sizeof(boxdata));

	if(a == NULL){
		fprintf(stderr, "Problem allocating memory for data structure\n");
		return 1;
	}
	else{
		printf("Initalization complete\n");
		usleep(15);
		system("clear");
	}

	srand(time(NULL));

	return 0;
}

int getarguments(boxdata *data){

	printf("Please enter the name for the csd file that will be made (exclude '.csd' file extension):\n");
	scanf("%s", data->filename);

    printf("Enter 1 to save the resulting file...\nEnter 2 if you'd like the file to play from the command line and delete itself\n");
    scanf("%d", &data->saveFlag);

    //append our filename with a .csd filetype
	strcat(data->filename, ".csd");

    system("clear");

	return 0;
}

void csdbuild(boxdata *data){

	double tmp;
    double gainChange;
    double panChange;
    int waveChange;
    int pluckChange;

	if(data->scorefile == NULL) {
		fprintf(stderr, "Problem opening file\n");
		}
	else{

		if(data->numchanges <= 0){
			data->numchanges = 1;
		}

		tmp = ((double)data->numchanges * 0.25) + 0.1;

		fprintf(data->scorefile, "<CsoundSynthesizer>\n");
		fprintf(data->scorefile, "<CsOptions>\n");
		fprintf(data->scorefile, "</CsOptions>\n");
		fprintf(data->scorefile, "<CsInstruments>\n");
		fprintf(data->scorefile, "sr = 44100\n");
		fprintf(data->scorefile, "ksmps = 32\n");
		fprintf(data->scorefile, "nchnls = 2\n");
		fprintf(data->scorefile, "0dbfs = 1\n");
        //init statements for all scores
		fprintf(data->scorefile, "gamixL init 0\ngamixR init 0\n");
		fprintf(data->scorefile, "gamix2L		init 0\n");
        fprintf(data->scorefile, "gamix2R 	init 0\n");
        fprintf(data->scorefile, "gadelay init 0\n");
        fprintf(data->scorefile, "garvbL init 0\n");
        fprintf(data->scorefile, "garvbR init 0\n");
        fprintf(data->scorefile, "ganzrvbL init 0\n");
        fprintf(data->scorefile, "ganzrvbR init 0\n");
        //i100 brightscore's primary instrument
		fprintf(data->scorefile, "instr 100\n");
		fprintf(data->scorefile, "kamp = ampdb (p4)\n");
		fprintf(data->scorefile, "ifrq = cpsmidinn (p5)\n");
		fprintf(data->scorefile, "irise = p6\n");
		fprintf(data->scorefile, "idec = p7\n");
		fprintf(data->scorefile, "ifn = 6\n");
		fprintf(data->scorefile, "ipanfrq = p8\n");
		fprintf(data->scorefile, "ares 	linen p4, p6, p3, p7\n");
		fprintf(data->scorefile, "kpan	poscil	.5, ipanfrq, -1\n");
		fprintf(data->scorefile, "asig 	poscil ares, ifrq, ifn\n");
		fprintf(data->scorefile, "kpanlfo	=		sqrt(kpan+.5)\n");
		fprintf(data->scorefile, "outs  	asig*kpanlfo, asig*(1-kpanlfo)\n");
		fprintf(data->scorefile, "gamixL = gamixL + asig\ngamixR = gamixR + asig\n");
		fprintf(data->scorefile, "endin\n");
        //i200 brightscore's bass instrument
		fprintf(data->scorefile, "instr 200\n");
		fprintf(data->scorefile, "kamp = p4\n");
		fprintf(data->scorefile, "kcps = cpsmidinn (p5)\n");
		fprintf(data->scorefile, "aenv linen p4, p6, p3, p7\n");
		fprintf(data->scorefile, "asig oscili aenv, kcps\n");
		fprintf(data->scorefile, "aLP butterlp asig, p8\n");
		fprintf(data->scorefile, "outs aLP, aLP\n");
		fprintf(data->scorefile, "endin\n");
        //i300 filter mod instrument for brightscore
		fprintf(data->scorefile, "instr 300\n");
        fprintf(data->scorefile, "kcps = cpsmidinn(p5)\n");
        fprintf(data->scorefile, "amod oscili p4, (kcps*1.5), 3\n");
        fprintf(data->scorefile, "acar oscili p4, kcps, 4\n");
        fprintf(data->scorefile, "amix1 = acar*amod\n");
        if(data->mainColor == 3){
            gainChange = 500;
            panChange = 0.25;
        }
        else{
            gainChange = 200;
            panChange = 1;
        }
        fprintf(data->scorefile, "klfo poscil %.2f, p8, 1\n", gainChange);
        fprintf(data->scorefile, "afilt zdf_ladder amix1, klfo+p9, 3\n");
        fprintf(data->scorefile, "aenv linen	afilt, p6, p3, p7\n");
        fprintf(data->scorefile, "kpan poscil 1, 3, 3\n");
        fprintf(data->scorefile, "apanL, apanR pan2 aenv, kpan*%.2f, 2\n", panChange);
        fprintf(data->scorefile, "outs apanL, apanR\n");
        fprintf(data->scorefile, "gamix2L = gamix2L+aenv\n");
        fprintf(data->scorefile, "gamix2R = gamix2R+aenv\n");
        fprintf(data->scorefile, "endin\n");
        //i400 rhythmic instrument for brightscore
        fprintf(data->scorefile, "instr 400\n");
        fprintf(data->scorefile, "kcps = cpsmidinn(p5)\n");
        fprintf(data->scorefile, "icps = cpsmidinn(p5)\n");
        //if blue score or green score has been selected, make the waveform and pluck type different
        if(data->mainColor == 7 || data->mainColor == 6){
            if(data->mainColor == 7){
                waveChange = 1;
                pluckChange = 1;
                gainChange = 0.6;
            }
            if(data->mainColor == 6){
                waveChange = 2;
                pluckChange = 1;
                gainChange = 0.8;
            }
        }
        else{
            waveChange = 3;
            pluckChange = 3;
            gainChange = 1;
        }
        fprintf(data->scorefile, "asig pluck p4*%.2f, kcps, icps, %d, %d, 0.1\n", gainChange, waveChange, pluckChange);
        fprintf(data->scorefile, "aenv linen asig, p6, p3, p7\n");
        fprintf(data->scorefile, "klfo poscil 350, p8, 3\n");
        fprintf(data->scorefile, "afilt zdf_ladder aenv, klfo+p9, 5\n");
        fprintf(data->scorefile, "outs afilt, afilt\n");
        fprintf(data->scorefile, "gadelay += afilt\n");
        fprintf(data->scorefile, "gamix2L += (afilt*0.25)\n");
        fprintf(data->scorefile, "gamix2R += (afilt*0.25)\n");
        fprintf(data->scorefile, "endin\n");
        //i500 main instrument in darkscore
        fprintf(data->scorefile, "instr 500\n");
        fprintf(data->scorefile, "kamp = ampdb(p4)\n");
        fprintf(data->scorefile, "kcps = cpsmidinn(p5)\n");
        fprintf(data->scorefile, "a1 poscil kamp, kcps*0.5, 1\n");
        fprintf(data->scorefile, "aenv1 linen a1, p6, p3, p7\n");
        fprintf(data->scorefile, "a2 poscil kamp, kcps*2.52, 1\n");
        fprintf(data->scorefile, "aenv2 linen a2, p6, p3, p7\n");
        fprintf(data->scorefile, "a3 poscil kamp, kcps*1.5, 1\n");
        fprintf(data->scorefile, "amix = aenv1 * aenv2\n");
        fprintf(data->scorefile, "amix2 = amix * a3         \n");
        fprintf(data->scorefile, "krand linrand p8\n");
        fprintf(data->scorefile, "afilt moogladder amix2, (krand+p9), 0.5\n");
        fprintf(data->scorefile, "kpenv expseg 1, p3*0.5, 3, p3*0.5, 1\n");
        fprintf(data->scorefile, "kpan poscil 1, kpenv, 1\n");
        fprintf(data->scorefile, "aL, aR pan2 afilt, kpan, 0\n");
        fprintf(data->scorefile, "outs aL, aR\n");
        fprintf(data->scorefile, "endin\n");
        //i600 darkscore's bass instrument
        fprintf(data->scorefile, "instr 600\n");
        fprintf(data->scorefile, "kamp = ampdb(p4)\n");
        fprintf(data->scorefile, "kcps = cpsmidinn(p5)\n");
        fprintf(data->scorefile, "a1 poscil kamp, kcps, 3\n");
        fprintf(data->scorefile, "aenv1 linen a1, p6, p3, p7\n");
        fprintf(data->scorefile, "a2 poscil kamp, kcps*0.5, 3\n");
        fprintf(data->scorefile, "aenv2 linen a2, p6, p3, p7\n");
        fprintf(data->scorefile, "amix = aenv1 * aenv2\n");
        fprintf(data->scorefile, "kenv expseg 1, p3*0.5, p8, p3*0.5, 1\n");
        fprintf(data->scorefile, "afilt butterlp amix, kenv+200\n");
        fprintf(data->scorefile, "aclip clip afilt, 2, 0.5\n");
        fprintf(data->scorefile, "outs aclip*0.2, aclip*0.2\n");
        fprintf(data->scorefile, "garvbL += aclip*0.3\n");
        fprintf(data->scorefile, "garvbR += aclip*0.3\n");
        fprintf(data->scorefile, "endin\n");
        //i700 wave terrain synth for darkscore
        fprintf(data->scorefile, "instr 700\n");
        fprintf(data->scorefile, "kdclk   linseg  0.1, p3/2, 0.5, p3/2, 0.1\n");
        fprintf(data->scorefile, "kcx     line    0.1, p3, 1.9\n");
        fprintf(data->scorefile, "krx     linseg  0.1, p3/2, 0.5, p3/2, 0.1\n");
        fprintf(data->scorefile, "kpch    line    cpsmidinn(p4), p3, p5 * cpsmidinn(p4)\n");
        fprintf(data->scorefile, "a1      wterrain    0.18, kpch, kcx, kcx, -krx, krx, p6, p7\n");
        fprintf(data->scorefile, "a1      dcblock a1\n");
        fprintf(data->scorefile, "amix = a1*kdclk\n");
        fprintf(data->scorefile, "outs amix, amix\n");
        fprintf(data->scorefile, "endin\n");
        //i800 pink noise, ring mod, and mild pitch modulation
        fprintf(data->scorefile, "instr 800\n");
		fprintf(data->scorefile, "kamp = ampdb(p4)\n");
		fprintf(data->scorefile, "kcps = cpsmidinn(p5)\n");
		fprintf(data->scorefile, "kmodfreq = p13\n");
		fprintf(data->scorefile, "aosc pinker\n");
		fprintf(data->scorefile, "aenv linen aosc, p6, (p3-0.5), p7\n");
		fprintf(data->scorefile, "anflt moogladder2 aenv, p8, p9\n");
		fprintf(data->scorefile, "ap1, ap2 pan2 anflt, p10, 1\n");
		fprintf(data->scorefile, "kmod poscil kamp*0.025, kmodfreq, 1\n");
		fprintf(data->scorefile, "asqr1 poscil kamp*0.6, kcps*.75, 3\n");
		fprintf(data->scorefile, "asqr2 poscil kamp*0.6, kcps*kmod, 3\n");
		fprintf(data->scorefile, "asqrmix = asqr1 * asqr2\n");
		fprintf(data->scorefile, "aenv2 linen asqrmix, p6, (p3-0.5), p7\n");
		fprintf(data->scorefile, "amflt moogladder2 aenv2, p11, p12\n");
		fprintf(data->scorefile, "apanL, apanR pan2 amflt, p10, 1\n");
        //if blue score has been detected, alter the volume of the noise oscillator in i800:
        if(data->mainColor == 7 || data->mainColor == 2){
            if(data->mainColor == 7){
                gainChange = 0.5;
            }
            if(data->mainColor == 2){
                gainChange = 0.25;
            }
        }
        else{
            gainChange = 0.95;
        }
		fprintf(data->scorefile, "aoscmixL = apanL + (ap1*%.2f)\n", gainChange);
		fprintf(data->scorefile, "aoscmixR = apanR + (ap2*%.2f)\n", gainChange);
		fprintf(data->scorefile, "outs aoscmixL, aoscmixR\n");
		fprintf(data->scorefile, "ganzrvbL += aoscmixL * 0.15\n");
		fprintf(data->scorefile, "ganzrvbR += aoscmixR * 0.15\n");
		fprintf(data->scorefile, "endin \n");
        //i1000 primary reverb for main instrument in brightscore
        fprintf(data->scorefile, "instr 1000\n");
		fprintf(data->scorefile, "aenvL linen gamixL, 12, p3, 10\n");
		fprintf(data->scorefile, "aenvR linen gamixR, 12, p3, 10\n");
		fprintf(data->scorefile, "aL, aR	reverbsc aenvL, aenvR, 1, %d, sr, 0.5, 1\n", (data->brightness * 1300));
		fprintf(data->scorefile, "outs	 	aL, aR\n");
		fprintf(data->scorefile, "clear gamixL\n");
		fprintf(data->scorefile, "clear gamixR\n");
		fprintf(data->scorefile, "endin\n");
		//i2000 reverberation for i800
		fprintf(data->scorefile, "instr 2000\n");
		fprintf(data->scorefile, "ainL, ainR reverbsc ganzrvbL, ganzrvbR, 1, 11000, sr, 2\n");
		fprintf(data->scorefile, "aenvL linen ainL, p4, p3, p5\n");
		fprintf(data->scorefile, "aenvR linen ainR, p4, p3, p5\n");
		fprintf(data->scorefile, "outs aenvL, aenvR\n");
		fprintf(data->scorefile, "clear ganzrvbL\n");
		fprintf(data->scorefile, "clear ganzrvbR\n");
		fprintf(data->scorefile, "endin\n");
        //i5000 global reverb for brightscore instruments
        fprintf(data->scorefile, "instr 5000\n");
        //allow for the pitch mod on revsc to be determined by art parameters
        fprintf(data->scorefile, "aL, aR reverbsc gamix2L, gamix2R, 0.9, 12500, sr, %d\n", (data->numchanges / 2));
        fprintf(data->scorefile, "outs (aL*0.5), (aR*0.5)\n");
        fprintf(data->scorefile, "clear gamix2L\n");
        fprintf(data->scorefile, "clear gamix2R\n");
        fprintf(data->scorefile, "	endin\n");
        //i6000 delay for brightscore instruments
        fprintf(data->scorefile, "	instr 6000\n");
        //allow for the variable delay times to be deteremined by an art parameter
        fprintf(data->scorefile, "adel multitap gadelay, %.2f, 0.8, %.2f, 0.6, %.2f, 0.4, %.2f, 0.35, %.2f, 0.3, %.2f, 0.25, %.2f, 0.2, %.2f, 0.15, %.2f, 0.1, %.2f, 0.01\n"
        												, tmp, (tmp * 2), (tmp * 4), (tmp * 6), (tmp * 8), (tmp * 16), (tmp * 32), (tmp * 64), (tmp * 128), (tmp * 256));
        fprintf(data->scorefile, "kpan poscil 1, 1, 3\n");
        fprintf(data->scorefile, "apL, apR pan2 adel, kpan, 3\n");
        fprintf(data->scorefile, "outs apL, apR\n");
        fprintf(data->scorefile, "clear gadelay\n");
        fprintf(data->scorefile, "	endin\n");
        //i7000 reverb for darkscore instruments
        fprintf(data->scorefile, "instr 7000\n");
        fprintf(data->scorefile, "aL, aR reverbsc garvbL, garvbR, 0.8, 12000, sr, 2\n");
        fprintf(data->scorefile, "aenvL linen aL, p4, p3, p5\n");
        fprintf(data->scorefile, "aenvR linen aR, p4, p3, p5\n");
        fprintf(data->scorefile, "outs aenvL*0.8, aenvR*0.8\n");
        fprintf(data->scorefile, "clear garvbL\n");
        fprintf(data->scorefile, "clear garvbR\n");
        fprintf(data->scorefile, "endin\n");
        //beginning of score tags
		fprintf(data->scorefile, "</CsInstruments>\n");
		fprintf(data->scorefile, "<CsScore>\n");
		fprintf(data->scorefile, "f6 0 16384 10 1 1   1   1    0.7 0.5   0.3  0.1\n");
		fprintf(data->scorefile, "f1 0 16384 10 1\n");
        fprintf(data->scorefile, "f2 0 16384 10 1 0.5 0.3 0.25 0.2 0.167 0.14 0.125 .111\n");
        fprintf(data->scorefile, "f3 0 16384 10 1 0   0.3 0    0.2 0     0.14 0     .111\n");
        fprintf(data->scorefile, "f4 0 16384 10 1 1   1   1    0.7 0.5   0.3  0.1\n");
    }
}

float randnum(){

	// Generate random panning values:
	float scaler = 4.0;
	float randresult;

	randresult = (rand()/(double)(RAND_MAX)) * scaler;

	return randresult;
}

int scales(boxdata *data){

	// lydian key steps
	int i, ii, iii, iv, v, vi, vii, viii;
	i 	= data->midi_note;
	ii 	= data->midi_note + 2;
	iii = data->midi_note + 4;
	iv 	= data->midi_note + 6;
	v 	= data->midi_note + 7;
	vi 	= data->midi_note + 9;
	vii = data->midi_note + 11;
	viii = data->midi_note + 12;
    // major key steps
    int doe, re, mi, fa, sol, la, ti, oct;
    doe = data->midi_note;
    re  = data->midi_note + 2;
    mi  = data->midi_note + 4;
    fa  = data->midi_note + 5;
    sol = data->midi_note + 7;
    la  = data->midi_note + 9;
    ti  = data->midi_note + 11;
    oct = data->midi_note + 12;
    //phrygian key steps
    int I, II, III, IV, V, VI, VII, VIII;
    I   = data->midi_note;
    II  = data->midi_note + 1;
    III = data->midi_note + 3;
    IV	= data->midi_note + 5;
    V	= data->midi_note + 7;
    VI 	= data->midi_note + 8;
    VII = data->midi_note + 10;
    VIII = data->midi_note + 12;

    //lydian
	if(data->keychoice == 3) {
		switch (rand() % (7 + 1 - 0) + 0) {
			case 0:
				data->result = i;
				break;
			case 1:
				data->result = ii;
				break;
			case 2:
				data->result = iii;
				break;
			case 3:
				data->result = iv;
				break;
			case 4:
				data->result = v;
				break;
			case 5:
				data->result = vi;
				break;
			case 6:
				data->result = vii;
				break;
			case 7:
				data->result = viii;
				break;
		}
	}

    //major
    if(data->keychoice == 2) {
        switch (rand() % (7 + 1 - 0) + 0) {
            case 0:
                data->result = doe;
                break;
            case 1:
                data->result = re;
                break;
            case 2:
                data->result = mi;
                break;
            case 3:
                data->result = fa;
                break;
            case 4:
                data->result = sol;
                break;
            case 5:
                data->result = la;
                break;
            case 6:
                data->result = ti;
                break;
            case 7:
                data->result = oct;
                break;
        }
    }

    //phrygian
    if(data->keychoice == 1){
        switch(rand() % (7 + 1 - 0) + 0){
            case 0:
                data->result = I;
                break;
            case 1:
                data->result = II;
                break;
            case 2:
                data->result = III;
                break;
            case 3:
                data->result = IV;
                break;
            case 4:
                data->result = V;
                break;
            case 5:
                data->result = VI;
                break;
            case 6:
                data->result = VII;
                break;
            case 7:
                data->result = VIII;
                break;
        }
    }

	return data->result;
}

void scorebuild(boxdata *data){

    if(data->scorefile == NULL) {
        fprintf(stderr, "Problem opening file\n");
    }

    int a;
    int b;
    int c;
    int lastBrghtnss;
    int lastColor;
    int deltaFlag;
    int prevScore;
    int count;
    int rnd;
    double nextpan;
    double rmod;
    double rate;
    double colorLength;
   	float lengthtotal;
    data->printflag = 0;
    data->totalLength = 0;
    data->scorenum = 0;

    switch(data->mainColor){
        case 1:
            //switch for grey score - noise pad, rhythm instrument, and dark bass
            switch(data->scorenum){
                case 0:
                    for(a = 0; a < data->length; a++){
                        //randomly generate our values for pitch, panning, and notelength
                        data->nextnote[a] = scales(data);
                        data->nextpan[a] = randnum();
                        data->notelength = (double)randnum() + 3;
                        data->testnote = data->nextnote[a];
            
                        //checking for duplicate notes
                        while (data->nextnote[a] == data->testnote) {
                            data->nextnote[a] = scales(data);
                        }
                    }
                    data->scorenum += 1;        
                case 1:
                    data->printflag = 0;
                    c = 0;
                    

                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 4.5;
                        //note checking 
                        if(data->nextnote[a] <= 60){
                            data->nextnote[a] += 12;
                        }
                        else if(data->nextnote[a] <= 55){
                            data->nextnote[a] += 24;
                        }
                        else if(data->nextnote[a] >= 85){
                            data->nextnote[a] -= 12;
                        }
                        else if((int)(data->nextnote[a]) >= 90){
                            data->nextnote[a] -= 24;
                        }
        
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                fprintf(data->scorefile, "i800 0 3 0.7 %d 2 3 %d 0.6 0.5 %d 0.5 %d\n", data->nextnote[a], (data->blockBrghtnss[c] * 1000), (data->blockBrghtnss[c] * 1000), (data->blockColor[c] * 5));
                            }
                            else{
                                fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f %d . . %d . %d\n", data->notelength, data->nextnote[a], (data->notelength - 1), (data->blockBrghtnss[c] * 1000), (data->blockBrghtnss[c] * 1000), (data->blockColor[c] * 5));
                            }

                            lastBrghtnss = data->blockBrghtnss[c];
                            lastColor = data->blockColor[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                if(lastColor == data->blockColor[c]){
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f . . . . . .\n", data->notelength, data->nextnote[a], (data->notelength - 1));
                                }
                                else{
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f . . . . . <\n", data->notelength, data->nextnote[a], (data->notelength - 1));
                                }
                            }
                            else{
                                if(lastColor == data->blockBrghtnss[c]){
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f < . . < . .\n", data->notelength, data->nextnote[a], (data->notelength - 1));
                                }   
                                else{
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f < . . < . <\n", data->notelength, data->nextnote[a], (data->notelength - 1));   
                                }
                            }
                        }

                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }

                    data->scorenum += 1;
                case 2:

                    lengthtotal = 0;
                    count = 0;
                    data->printflag = 0;
                    c = 0;          
                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 3;
                        rate = 2;
                        int pitch[3];
                        int z;
                        //fill up array indexes for the groups of pitches played by the rhythm instrument, then constrain them to their frequency range
                        for(z = 0; z < 3; z++){
                            pitch[z] = scales(data);
                            if(pitch[z] >= 65){
                                pitch[z] -= 12;
                            }
                            if(pitch[z] >= 70){
                                pitch[z] -= 24;
                            }
                        }
                        int rnd = ((rand() % 6) + 1);
                        count++;
                        //used to calculate the total amount of time taken up by the rhythm instrument, so that it doesn't play longer than the other instruments
                        lengthtotal += data->notelength;
                        //determine when to change the rate of the rhythm
                        if(count % rnd == 0){
                            rate = rate / 2;
                        }           
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the rhythm instrument
                                fprintf(data->scorefile, "i400 0 %.2f 0.5 %d 0 0.1 1 %d\n", rate, (data->midi_note + 12), (data->blockBrghtnss[0] * 500));
                            }
                            else{
                                fprintf(data->scorefile, "i400 + %.2f 0.5 %d . . . %d\n", rate, (data->midi_note + 12), (data->blockBrghtnss[c] * 500));
                            }           
                            if(data->blockColor[c] >= 5){
                                rate = 2;
                            }
                            else{
                                rate = 1;
                            }               
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                //additional lines for the rhythm instrument
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (data->nextnote[a] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[0] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[1] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[2] + 12));
                            }   
                            else{
                                //additional lines for the rhythm instrument
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (data->nextnote[a] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[0] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[1] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[2] + 12));
                            }
                        }           
                        data->totalLength += (data->notelength);
                        data->printflag++;
                       
                        if((data-> printflag == data->length) && (lengthtotal <= (data->totalLength - 30))){
                            a--;
                        }
                    }
                    data->scorenum += 1;
                case 3:
                    data->printflag = 0;
                    c = 0;
                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 4.5;
                        //constraining the note numbers to their frequency ranges
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        else if(data->nextnote[a] <= 25){
                            data->nextnote[a] += 24;
                        }
                        else if(data->nextnote[a] >= 60){
                            data->nextnote[a] -= 24;
                        }
                        else if((data->nextnote[a]) >= 71){
                            data->nextnote[a] -= 36;
                        }           
                        
                        
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the dark bass instrument
                                fprintf(data->scorefile, "i600 0 4 0.15 %d 1 3.8 %d\n", (data->midi_note), (data->blockBrghtnss[0] * 75));
                            }
                            else{
                                fprintf(data->scorefile, "i600 + %.2f . %d . %.2f %d\n", data->notelength, (data->nextnote[a]), (data->notelength - 0.5), (data->blockBrghtnss[0] * 75));
                            }    
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                fprintf(data->scorefile, "i600 + %.2f . %d . %.2f .\n", data->notelength, (data->nextnote[a]), (data->notelength - 0.5));
                            }   
                            else{
                                fprintf(data->scorefile, "i600 + %.2f . %d . %.2f <\n", data->notelength, (data->nextnote[a]), (data->notelength - 0.5));
                            }
                        }               
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }      

                    data->scorenum += 1;
                case 4:
                    fprintf(data->scorefile, "i1000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i2000 0 %.2f 1 4\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i5000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i6000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i7000 0 %.2f 2 3\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "e\n");
                    fprintf(data->scorefile, "</CsScore>\n</CsoundSynthesizer>\n");
                    break;
            }
            break;
        case 2:
            //switch for cyan score - bright score pad, bright score bass, and noise pad
            switch(data->scorenum){
                case 0:
                    for(a = 0; a < data->length; a++){
                        //randomly generate our values for pitch, panning, and notelength
                        data->nextnote[a] = scales(data);
                        data->nextpan[a] = randnum();
                        data->notelength = (double)randnum() + 3;
                        data->testnote = data->nextnote[a];
            
                        //checking for duplicate notes
                        while (data->nextnote[a] == data->testnote) {
                            data->nextnote[a] = scales(data);
                        }
                    }
                    data->scorenum += 1;        
                case 1:

                    data->printflag = 0;
                    c = 0;
                     
                    //print the first line of the score for the pad instrument;
                    fprintf(data->scorefile, "i100 0 5 0.05 %d 2 3.5  %.2f\n", data->midi_note + 12, randnum());
                    for(a = 0; a < data->length; a++){
                        data->printflag++;
                        data->notelength = (double)randnum() + 4;
                    
                        //constraining the note numbers to their frequency range
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        if(data->nextnote[a] >= 85){
                            data->nextnote[a] -= 12;
                        }
                        
                        //print the next line for the pad
                        fprintf(data->scorefile, "i100 + %.2f . %d . %.2f %.2f\n", data->notelength, data->nextnote[a], (data->notelength - 0.5), data->nextpan[a]);            
                        //this is used to calculate the total length of the score, it is divided by 3 at the end because it counts for all three instruments
                        data->totalLength += (data->notelength + 1);
        
                    }
                    data->scorenum += 1;
                case 2:

                    data->printflag = 0;
                    c = 0;
                    
                    for(a = 0; a < (data->length); a++){
                        data->notelength = (double)randnum() + 4.5;         
                        //constraining the note numbers to their frequency ranges
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        else if(data->nextnote[a] <= 25){
                            data->nextnote[a] += 24;
                        }
                        else if(data->nextnote[a] >= 60){
                            data->nextnote[a] -= 24;
                        }
                        else if((int)(data->nextnote[a]) >= 71){
                            data->nextnote[a] -= 36;
                        }           
                        if((data->printflag % data->numBlocks) == 0){
        
                            if(data->printflag == 0){
                                //print the first line of the score for the bass instrument
                                fprintf(data->scorefile, "i200 0 4 0.15 %d 1 3.8 %d\n", (data->midi_note - 12), (data->blockBrghtnss[c] * 90));
                            }
                            else{
                                fprintf(data->scorefile, "i200 + %.2f . %d . %.2f %d\n", data->notelength, (data->nextnote[a] - 12), (data->notelength - 0.5), (data->blockBrghtnss[c] * 80));
                            }           
                            lastBrghtnss = data->blockBrghtnss[c];
                            lastColor = data->blockColor[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                fprintf(data->scorefile, "i200 + %.2f . %d . %.2f .\n", data->notelength, (data->nextnote[a] - 12), (data->notelength - 0.5));
                            }   
                            else{
                                fprintf(data->scorefile, "i200 + %.2f . %d . %.2f <\n", data->notelength, (data->nextnote[a] - 12), (data->notelength - 0.5));
                            }
                        }           
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }           
                    data->scorenum += 1;
                case 3:
                    data->printflag = 0;
                    c = 0;
                    

                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 4.5;
                        //note checking 
                        if(data->nextnote[a] <= 60){
                            data->nextnote[a] += 12;
                        }
                        else if(data->nextnote[a] <= 55){
                            data->nextnote[a] += 24;
                        }
                        else if(data->nextnote[a] >= 85){
                            data->nextnote[a] -= 12;
                        }
                        else if((int)(data->nextnote[a]) >= 90){
                            data->nextnote[a] -= 24;
                        }
        
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                fprintf(data->scorefile, "i800 0 3 0.7 %d 2 3 %d 0.6 0.5 %d 0.5 %d\n", data->nextnote[a], (data->blockBrghtnss[c] * 1000), (data->blockBrghtnss[c] * 1000), (data->blockColor[c] * 5));
                            }
                            else{
                                fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f %d . . %d . %d\n", data->notelength, data->nextnote[a], (data->notelength - 1), (data->blockBrghtnss[c] * 1000), (data->blockBrghtnss[c] * 1000), (data->blockColor[c] * 5));
                            }
                            lastBrghtnss = data->blockBrghtnss[c];
                            lastColor = data->blockColor[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                if(lastColor == data->blockColor[c]){
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f . . . . . .\n", data->notelength, data->nextnote[a], (data->notelength - 1));
                                }
                                else{
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f . . . . . <\n", data->notelength, data->nextnote[a], (data->notelength - 1));
                                }
                            }
                            else{
                                if(lastColor == data->blockBrghtnss[c]){
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f < . . < . .\n", data->notelength, data->nextnote[a], (data->notelength - 1));
                                }   
                                else{
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f < . . < . <\n", data->notelength, data->nextnote[a], (data->notelength - 1));   
                                }
                            }
                        }
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }

                    data->scorenum += 1;
                case 4:
                    fprintf(data->scorefile, "i1000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i2000 0 %.2f 1 4\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i5000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i6000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i7000 0 %.2f 2 3\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "e\n");
                    fprintf(data->scorefile, "</CsScore>\n</CsoundSynthesizer>\n");
                    break;
            }
            break;
        case 3:
            //switch for magenta score - filter mod instrument, dark pad, wave terrain, and rhythm instrument
            switch(data->scorenum){
                case 0:
                    for(a = 0; a < data->length; a++){
                        //randomly generate our values for pitch, panning, and notelength
                        data->nextnote[a] = scales(data);
                        data->nextpan[a] = randnum();
                        data->notelength = (double)randnum() + 3;
                        data->testnote = data->nextnote[a];
            
                        //checking for duplicate notes
                        while (data->nextnote[a] == data->testnote) {
                            data->nextnote[a] = scales(data);
                        }
                    }
                    data->scorenum += 1;        
                case 1:

                    rnd = ((rand() % 7) + 1);
                    c = 0;
                    data->printflag = 0;            
                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 3;
                        rmod = 1;
                        count++;
                        //determine when to change the filter mod rate
                        if(count % rnd == 0){
                            rmod = 2;
                        }
                        //constraining the note numbers to their frequency range
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        else if(data->nextnote[a] <= 25){
                            data->nextnote[a] += 24;
                        }
                        else if(data->nextnote[a] >= 60){
                            data->nextnote[a] -= 12;
                        }
                        else if(data->nextnote[a] >= 75){
                            data->nextnote[a] -= 24;
                        }           
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the filter mod instrument;
                            fprintf(data->scorefile, "i300 0 5 0.7 %d 0.02 5 1 %d\n", data->midi_note, (data->blockBrghtnss[0] * 1000));
                            }
                            else{
                                fprintf(data->scorefile, "i300 + %.2f . %d . %.1f %.1f %d\n", data->notelength, data->nextnote[a], (data->notelength - 1), rmod, data->blockBrghtnss[c] * 1000);
                            }    
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                fprintf(data->scorefile, "i300 + %.2f . %d . %.1f %.1f .\n", data->notelength, data->nextnote[a], (data->notelength - 1), rmod);
                            }   
                            else{
                                fprintf(data->scorefile, "i300 + %.2f . %d . %.1f %.1f <\n", data->notelength, data->nextnote[a], (data->notelength - 1), rmod);
                            }
                        }                    
                        
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }           
                    data->scorenum += 1;
                case 2:
                    data->printflag = 0;
                    c = 0;

                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 4;
                    
                        //constraining the note numbers to their frequency range
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        if(data->nextnote[a] >= 85){
                            data->nextnote[a] -= 12;
                        }
                        
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the dark pad instrument;
                                fprintf(data->scorefile, "i500 0 5 0.5 %d 2 3.5 %d %d\n", data->midi_note + 12, (rand() % 100) + 100, (data->blockBrghtnss[0] * 175));
                            }
                            else{
                                fprintf(data->scorefile, "i500 + %.2f . %d . %.2f %d %d\n", data->notelength, data->nextnote[a] + 12, (data->notelength - 0.5), (rand() % 100) + 100, (data->blockBrghtnss[c] * 175));
                            }    
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                fprintf(data->scorefile, "i500 + %.2f . %d . %.2f %d .\n", data->notelength, data->nextnote[a] + 12, (data->notelength - 0.5), (rand() % 100) + 100);
                            }   
                            else{
                                fprintf(data->scorefile, "i500 + %.2f . %d . %.2f %d <\n", data->notelength, data->nextnote[a] + 12, (data->notelength - 0.5), (rand() % 100) + 100);
                            }
                        }               
                        //this is used to calculate the total length of the score, it is divided by 3 at the end because it counts for all three instruments
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }
            
                    data->scorenum += 1;
                case 3:

                    lengthtotal = 0;
                    count = 0;
                    data->printflag = 0;
                    c = 0;          
                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 3;
                        rate = 2;
                        int pitch[3];
                        int z;
                        //fill up array indexes for the groups of pitches played by the rhythm instrument, then constrain them to their frequency range
                        for(z = 0; z < 3; z++){
                            pitch[z] = scales(data);
                            if(pitch[z] >= 65){
                                pitch[z] -= 12;
                            }
                            if(pitch[z] >= 70){
                                pitch[z] -= 24;
                            }
                        }
                        int rnd = ((rand() % 6) + 1);
                        count++;
                        //used to calculate the total amount of time taken up by the rhythm instrument, so that it doesn't play longer than the other instruments
                        lengthtotal += data->notelength;
                        //determine when to change the rate of the rhythm
                        if(count % rnd == 0){
                            rate = rate / 2;
                        }           
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the rhythm instrument
                                fprintf(data->scorefile, "i400 0 %.2f 0.5 %d 0 0.1 1 %d\n", rate, (data->midi_note + 12), (data->blockBrghtnss[0] * 500));
                            }
                            else{
                                fprintf(data->scorefile, "i400 + %.2f 0.5 %d . . . %d\n", rate, (data->midi_note + 12), (data->blockBrghtnss[c] * 500));
                            }           
                            if(data->blockColor[c] >= 5){
                                rate = 2;
                            }
                            else{
                                rate = 1;
                            }               
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                //additional lines for the rhythm instrument
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (data->nextnote[a] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[0] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[1] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[2] + 12));
                            }   
                            else{
                                //additional lines for the rhythm instrument
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (data->nextnote[a] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[0] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[1] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[2] + 12));
                            }
                        }           
                        data->totalLength += (data->notelength);
                        data->printflag++;
                       
                        if((data-> printflag == data->length) && (lengthtotal <= (data->totalLength - 30))){
                            a--;
                        }
                    }
                    data->scorenum += 1;
                case 4:
                    fprintf(data->scorefile, "i1000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i2000 0 %.2f 1 4\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i5000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i6000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i7000 0 %.2f 2 3\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "e\n");
                    fprintf(data->scorefile, "</CsScore>\n</CsoundSynthesizer>\n");
                    break;
            }
            break;
        case 4:
            //switch for yellow score - bright score pad, bright score bass, wave terrain lead, rhythm instrument
            switch(data->scorenum){
                case 0:
                    for(a = 0; a < data->length; a++){
                        //randomly generate our values for pitch, panning, and notelength
                        data->nextnote[a] = scales(data);
                        data->nextpan[a] = randnum();
                        data->notelength = (double)randnum() + 3;
                        data->testnote = data->nextnote[a];
            
                        //checking for duplicate notes
                        while (data->nextnote[a] == data->testnote) {
                            data->nextnote[a] = scales(data);
                        }
                    }
                    data->scorenum += 1;        
                case 1:

                    data->printflag = 0;
                    c = 0;
                     
                    //print the first line of the score for the pad instrument;
                    fprintf(data->scorefile, "i100 0 5 0.05 %d 2 3.5  %.2f\n", data->midi_note + 12, randnum());
                    for(a = 0; a < data->length; a++){
                        data->printflag++;
                        data->notelength = (double)randnum() + 4;
                    
                        //constraining the note numbers to their frequency range
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        if(data->nextnote[a] >= 85){
                            data->nextnote[a] -= 12;
                        }
                        
                        //print the next line for the pad
                        fprintf(data->scorefile, "i100 + %.2f . %d . %.2f %.2f\n", data->notelength, data->nextnote[a], (data->notelength - 0.5), data->nextpan[a]);            
                        //this is used to calculate the total length of the score, it is divided by 3 at the end because it counts for all three instruments
                        data->totalLength += (data->notelength + 1);
        
                    }
                    data->scorenum += 1;
                case 2:

                    data->printflag = 0;
                    c = 0;
                    
                    for(a = 0; a < (data->length); a++){
                        data->notelength = (double)randnum() + 4.5;         
                        //constraining the note numbers to their frequency ranges
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        else if(data->nextnote[a] <= 25){
                            data->nextnote[a] += 24;
                        }
                        else if(data->nextnote[a] >= 60){
                            data->nextnote[a] -= 24;
                        }
                        else if((int)(data->nextnote[a]) >= 71){
                            data->nextnote[a] -= 36;
                        }           
                        if((data->printflag % data->numBlocks) == 0){
        
                            if(data->printflag == 0){
                                //print the first line of the score for the bass instrument
                                fprintf(data->scorefile, "i200 0 4 0.15 %d 1 3.8 %d\n", (data->midi_note - 12), (data->blockBrghtnss[c] * 90));
                            }
                            else{
                                fprintf(data->scorefile, "i200 + %.2f . %d . %.2f %d\n", data->notelength, (data->nextnote[a] - 12), (data->notelength - 0.5), (data->blockBrghtnss[c] * 80));
                            }           
                            lastBrghtnss = data->blockBrghtnss[c];
                            lastColor = data->blockColor[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                fprintf(data->scorefile, "i200 + %.2f . %d . %.2f .\n", data->notelength, (data->nextnote[a] - 12), (data->notelength - 0.5));
                            }   
                            else{
                                fprintf(data->scorefile, "i200 + %.2f . %d . %.2f <\n", data->notelength, (data->nextnote[a] - 12), (data->notelength - 0.5));
                            }
                        }           
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }           
                    data->scorenum += 1;
                case 3:

                    //print the first line of the score for the wave terrain instrument;
                    fprintf(data->scorefile, "i700 0 5 %d 1 3 2\n", data->midi_note);
                    
                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 4;
                    
                        //constraining the note numbers to their frequency range
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 24;
                        }
                        if(data->nextnote[a] >= 85){
                            data->nextnote[a] -= 12;
                        }
                    
                        //print the next line for the pad
                        fprintf(data->scorefile, "i700 + %.2f %d 1 3 2\n", data->notelength, data->nextnote[a]);
                    
                        //this is used to calculate the total length of the score, it is divided by 3 at the end because it counts for all three instruments
                        data->totalLength += (data->notelength + 1);
                    }
                    
                    data->scorenum += 1;
                case 4:

                    lengthtotal = 0;
                    count = 0;
                    data->printflag = 0;
                    c = 0;          
                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 3;
                        rate = 2;
                        int pitch[3];
                        int z;
                        //fill up array indexes for the groups of pitches played by the rhythm instrument, then constrain them to their frequency range
                        for(z = 0; z < 3; z++){
                            pitch[z] = scales(data);
                            if(pitch[z] >= 65){
                                pitch[z] -= 12;
                            }
                            if(pitch[z] >= 70){
                                pitch[z] -= 24;
                            }
                        }
                        int rnd = ((rand() % 6) + 1);
                        count++;
                        //used to calculate the total amount of time taken up by the rhythm instrument, so that it doesn't play longer than the other instruments
                        lengthtotal += data->notelength;
                        //determine when to change the rate of the rhythm
                        if(count % rnd == 0){
                            rate = rate / 2;
                        }           
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the rhythm instrument
                                fprintf(data->scorefile, "i400 0 %.2f 0.5 %d 0 0.1 1 %d\n", rate, (data->midi_note + 12), (data->blockBrghtnss[0] * 500));
                            }
                            else{
                                fprintf(data->scorefile, "i400 + %.2f 0.5 %d . . . %d\n", rate, (data->midi_note + 12), (data->blockBrghtnss[c] * 500));
                            }           
                            if(data->blockColor[c] >= 5){
                                rate = 2;
                            }
                            else{
                                rate = 1;
                            }               
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                //additional lines for the rhythm instrument
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (data->nextnote[a] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[0] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[1] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[2] + 12));
                            }   
                            else{
                                //additional lines for the rhythm instrument
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (data->nextnote[a] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[0] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[1] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[2] + 12));
                            }
                        }           
                        data->totalLength += (data->notelength);
                        data->printflag++;
                       
                        if((data-> printflag == data->length) && (lengthtotal <= (data->totalLength - 30))){
                            a--;
                        }
                    }
                    data->scorenum += 1;
                case 5:
                    fprintf(data->scorefile, "i1000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i2000 0 %.2f 1 4\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i5000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i6000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i7000 0 %.2f 2 3\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "e\n");
                    fprintf(data->scorefile, "</CsScore>\n</CsoundSynthesizer>\n");
                    break;
            }
            break;
        case 5:
            //switch for red score - dark pad, dark bass, filter mod instrument 
            switch(data->scorenum){
                case 0:
                    for(a = 0; a < data->length; a++){
                        //randomly generate our values for pitch, panning, and notelength
                        data->nextnote[a] = scales(data);
                        data->nextpan[a] = randnum();
                        data->notelength = (double)randnum() + 3;
                        data->testnote = data->nextnote[a];
            
                        //checking for duplicate notes
                        while (data->nextnote[a] == data->testnote) {
                            data->nextnote[a] = scales(data);
                        }
                    }
                    data->scorenum += 1;        
                case 1:
                    data->printflag = 0;
                    c = 0;

                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 4;
                    
                        //constraining the note numbers to their frequency range
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        if(data->nextnote[a] >= 85){
                            data->nextnote[a] -= 12;
                        }
                        
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the dark pad instrument;
                                fprintf(data->scorefile, "i500 0 5 0.5 %d 2 3.5 %d %d\n", data->midi_note + 12, (rand() % 100) + 100, (data->blockBrghtnss[0] * 175));
                            }
                            else{
                                fprintf(data->scorefile, "i500 + %.2f . %d . %.2f %d %d\n", data->notelength, data->nextnote[a] + 12, (data->notelength - 0.5), (rand() % 100) + 100, (data->blockBrghtnss[c] * 175));
                            }    
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                fprintf(data->scorefile, "i500 + %.2f . %d . %.2f %d .\n", data->notelength, data->nextnote[a] + 12, (data->notelength - 0.5), (rand() % 100) + 100);
                            }   
                            else{
                                fprintf(data->scorefile, "i500 + %.2f . %d . %.2f %d <\n", data->notelength, data->nextnote[a] + 12, (data->notelength - 0.5), (rand() % 100) + 100);
                            }
                        }               
                        //this is used to calculate the total length of the score, it is divided by 3 at the end because it counts for all three instruments
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }
            
                    data->scorenum += 1;
                case 2:
                    data->printflag = 0;
                    c = 0;
                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 4.5;
                        //constraining the note numbers to their frequency ranges
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        else if(data->nextnote[a] <= 25){
                            data->nextnote[a] += 24;
                        }
                        else if(data->nextnote[a] >= 60){
                            data->nextnote[a] -= 24;
                        }
                        else if((data->nextnote[a]) >= 71){
                            data->nextnote[a] -= 36;
                        }           
                        
                        
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the dark bass instrument
                                fprintf(data->scorefile, "i600 0 4 0.15 %d 1 3.8 %d\n", (data->midi_note), (data->blockBrghtnss[0] * 75));
                            }
                            else{
                                fprintf(data->scorefile, "i600 + %.2f . %d . %.2f %d\n", data->notelength, (data->nextnote[a]), (data->notelength - 0.5), (data->blockBrghtnss[0] * 75));
                            }    
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                fprintf(data->scorefile, "i600 + %.2f . %d . %.2f .\n", data->notelength, (data->nextnote[a]), (data->notelength - 0.5));
                            }   
                            else{
                                fprintf(data->scorefile, "i600 + %.2f . %d . %.2f <\n", data->notelength, (data->nextnote[a]), (data->notelength - 0.5));
                            }
                        }               
        
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }        

                    data->scorenum += 1;
                case 3:

                    rnd = ((rand() % 7) + 1);
                    c = 0;
                    data->printflag = 0;            
                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 3;
                        rmod = 1;
                        count++;
                        //determine when to change the filter mod rate
                        if(count % rnd == 0){
                            rmod = 2;
                        }
                        //constraining the note numbers to their frequency range
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        else if(data->nextnote[a] <= 25){
                            data->nextnote[a] += 24;
                        }
                        else if(data->nextnote[a] >= 60){
                            data->nextnote[a] -= 12;
                        }
                        else if(data->nextnote[a] >= 75){
                            data->nextnote[a] -= 24;
                        }           
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the filter mod instrument;
                            fprintf(data->scorefile, "i300 0 5 0.7 %d 0.02 5 1 %d\n", data->midi_note, (data->blockBrghtnss[0] * 500));
                            }
                            else{
                                fprintf(data->scorefile, "i300 + %.2f . %d . %.1f %.1f %d\n", data->notelength, data->nextnote[a], (data->notelength - 1), rmod, data->blockBrghtnss[c] * 500);
                            }    
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                fprintf(data->scorefile, "i300 + %.2f . %d . %.1f %.1f .\n", data->notelength, data->nextnote[a], (data->notelength - 1), rmod);
                            }   
                            else{
                                fprintf(data->scorefile, "i300 + %.2f . %d . %.1f %.1f <\n", data->notelength, data->nextnote[a], (data->notelength - 1), rmod);
                            }
                        }                    
                        
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }           
                    data->scorenum += 1;
                case 4:
                    fprintf(data->scorefile, "i1000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i2000 0 %.2f 1 4\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i5000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i6000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i7000 0 %.2f 2 3\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "e\n");
                    fprintf(data->scorefile, "</CsScore>\n</CsoundSynthesizer>\n");
                    break;
            }
            break;
        case 6:
            //switch for green score - bright score pad, dark bass, noise pad, and rhythm instrument
            switch(data->scorenum){
                case 0:
                    for(a = 0; a < data->length; a++){
                        //randomly generate our values for pitch, panning, and notelength
                        data->nextnote[a] = scales(data);
                        data->nextpan[a] = randnum();
                        data->notelength = (double)randnum() + 3;
                        data->testnote = data->nextnote[a];
            
                        //checking for duplicate notes
                        while (data->nextnote[a] == data->testnote) {
                            data->nextnote[a] = scales(data);
                        }
                    }
                    data->scorenum += 1;        
                case 1:

                    data->printflag = 0;
                    c = 0;
                     
                    //print the first line of the score for the pad instrument;
                    fprintf(data->scorefile, "i100 0 5 0.05 %d 2 3.5  %.2f\n", data->midi_note + 12, randnum());
                    for(a = 0; a < data->length; a++){
                        data->printflag++;
                        data->notelength = (double)randnum() + 4;
                    
                        //constraining the note numbers to their frequency range
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        if(data->nextnote[a] >= 85){
                            data->nextnote[a] -= 12;
                        }
                        
                        //print the next line for the pad
                        fprintf(data->scorefile, "i100 + %.2f . %d . %.2f %.2f\n", data->notelength, data->nextnote[a], (data->notelength - 0.5), data->nextpan[a]);            
                        //this is used to calculate the total length of the score, it is divided by 3 at the end because it counts for all three instruments
                        data->totalLength += (data->notelength + 1);
        
                    }
                    data->scorenum += 1;
                case 2:
                    data->printflag = 0;
                    c = 0;
                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 4.5;
                        //constraining the note numbers to their frequency ranges
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        else if(data->nextnote[a] <= 25){
                            data->nextnote[a] += 24;
                        }
                        else if(data->nextnote[a] >= 60){
                            data->nextnote[a] -= 24;
                        }
                        else if((data->nextnote[a]) >= 71){
                            data->nextnote[a] -= 36;
                        }           
                        
                        
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the dark bass instrument
                                fprintf(data->scorefile, "i600 0 4 0.15 %d 1 3.8 %d\n", (data->midi_note), (data->blockBrghtnss[0] * 75));
                            }
                            else{
                                fprintf(data->scorefile, "i600 + %.2f . %d . %.2f %d\n", data->notelength, (data->nextnote[a]), (data->notelength - 0.5), (data->blockBrghtnss[0] * 75));
                            }    
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                fprintf(data->scorefile, "i600 + %.2f . %d . %.2f .\n", data->notelength, (data->nextnote[a]), (data->notelength - 0.5));
                            }   
                            else{
                                fprintf(data->scorefile, "i600 + %.2f . %d . %.2f <\n", data->notelength, (data->nextnote[a]), (data->notelength - 0.5));
                            }
                        }               
        
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }           
                    data->scorenum += 1;
                case 3:
                    data->printflag = 0;
                    c = 0;
                    

                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 4.5;
                        //note checking 
                        if(data->nextnote[a] <= 60){
                            data->nextnote[a] += 12;
                        }
                        else if(data->nextnote[a] <= 55){
                            data->nextnote[a] += 24;
                        }
                        else if(data->nextnote[a] >= 85){
                            data->nextnote[a] -= 12;
                        }
                        else if((int)(data->nextnote[a]) >= 90){
                            data->nextnote[a] -= 24;
                        }
        
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                fprintf(data->scorefile, "i800 0 3 0.7 %d 2 3 %d 0.6 0.5 %d 0.5 %d\n", data->nextnote[a], (data->blockBrghtnss[c] * 1000), (data->blockBrghtnss[c] * 1000), (data->blockColor[c] * 5));
                            }
                            else{
                                fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f %d . . %d . %d\n", data->notelength, data->nextnote[a], (data->notelength - 1), (data->blockBrghtnss[c] * 1000), (data->blockBrghtnss[c] * 1000), (data->blockColor[c] * 5));
                            }
                            lastBrghtnss = data->blockBrghtnss[c];
                            lastColor = data->blockColor[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                if(lastColor == data->blockColor[c]){
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f . . . . . .\n", data->notelength, data->nextnote[a], (data->notelength - 1));
                                }
                                else{
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f . . . . . <\n", data->notelength, data->nextnote[a], (data->notelength - 1));
                                }
                            }
                            else{
                                if(lastColor == data->blockBrghtnss[c]){
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f < . . < . .\n", data->notelength, data->nextnote[a], (data->notelength - 1));
                                }   
                                else{
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f < . . < . <\n", data->notelength, data->nextnote[a], (data->notelength - 1));   
                                }
                            }
                        }
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }

                    data->scorenum += 1;
                case 4:

                    lengthtotal = 0;
                    count = 0;
                    data->printflag = 0;
                    c = 0;          
                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 3;
                        rate = 2;
                        int pitch[3];
                        int z;
                        //fill up array indexes for the groups of pitches played by the rhythm instrument, then constrain them to their frequency range
                        for(z = 0; z < 3; z++){
                            pitch[z] = scales(data);
                            if(pitch[z] >= 65){
                                pitch[z] -= 12;
                            }
                            if(pitch[z] >= 70){
                                pitch[z] -= 24;
                            }
                        }
                        int rnd = ((rand() % 6) + 1);
                        count++;
                        //used to calculate the total amount of time taken up by the rhythm instrument, so that it doesn't play longer than the other instruments
                        lengthtotal += data->notelength;
                        //determine when to change the rate of the rhythm
                        if(count % rnd == 0){
                            rate = rate / 2;
                        }           
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the rhythm instrument
                                fprintf(data->scorefile, "i400 0 %.2f 0.5 %d 0 0.1 1 %d\n", rate, (data->midi_note + 12), (data->blockBrghtnss[0] * 500));
                            }
                            else{
                                fprintf(data->scorefile, "i400 + %.2f 0.5 %d . . . %d\n", rate, (data->midi_note + 12), (data->blockBrghtnss[c] * 500));
                            }           
                            if(data->blockColor[c] >= 5){
                                rate = 2;
                            }
                            else{
                                rate = 1;
                            }               
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                //additional lines for the rhythm instrument
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (data->nextnote[a] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[0] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[1] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[2] + 12));
                            }   
                            else{
                                //additional lines for the rhythm instrument
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (data->nextnote[a] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[0] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[1] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[2] + 12));
                            }
                        }           
                        data->totalLength += (data->notelength);
                        data->printflag++;
                       
                        if((data-> printflag == data->length) && (lengthtotal <= (data->totalLength - 30))){
                            a--;
                        }
                    }
                    data->scorenum += 1;
                case 5:
                    fprintf(data->scorefile, "i1000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i2000 0 %.2f 1 4\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i5000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i6000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i7000 0 %.2f 2 3\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "e\n");
                    fprintf(data->scorefile, "</CsScore>\n</CsoundSynthesizer>\n");
                    break;
            }
            break;   
        case 7:
            //switch for blue score - dark pad, noise pad, dark bass, and rhythm instrument
            switch(data->scorenum){
                case 0:
                    for(a = 0; a < data->length; a++){
                        //randomly generate our values for pitch, panning, and notelength
                        data->nextnote[a] = scales(data);
                        data->nextpan[a] = randnum();
                        data->notelength = (double)randnum() + 3;
                        data->testnote = data->nextnote[a];
            
                        //checking for duplicate notes
                        while (data->nextnote[a] == data->testnote) {
                            data->nextnote[a] = scales(data);
                        }
                    }
                    data->scorenum += 1;        
                case 1:
                    data->printflag = 0;
                    c = 0;

                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 4;
                    
                        //constraining the note numbers to their frequency range
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        if(data->nextnote[a] >= 85){
                            data->nextnote[a] -= 12;
                        }
                        
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the dark pad instrument;
                                fprintf(data->scorefile, "i500 0 5 0.5 %d 2 3.5 %d %d\n", data->midi_note + 12, (rand() % 100) + 100, (data->blockBrghtnss[0] * 175));
                            }
                            else{
                                fprintf(data->scorefile, "i500 + %.2f . %d . %.2f %d %d\n", data->notelength, data->nextnote[a] + 12, (data->notelength - 0.5), (rand() % 100) + 100, (data->blockBrghtnss[c] * 175));
                            }    
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                fprintf(data->scorefile, "i500 + %.2f . %d . %.2f %d .\n", data->notelength, data->nextnote[a] + 12, (data->notelength - 0.5), (rand() % 100) + 100);
                            }   
                            else{
                                fprintf(data->scorefile, "i500 + %.2f . %d . %.2f %d <\n", data->notelength, data->nextnote[a] + 12, (data->notelength - 0.5), (rand() % 100) + 100);
                            }
                        }               
                        //this is used to calculate the total length of the score, it is divided by 3 at the end because it counts for all three instruments
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }
            
                    data->scorenum += 1;
                case 2:
                    data->printflag = 0;
                    c = 0;
                    

                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 4.5;
                        //note checking 
                        if(data->nextnote[a] <= 60){
                            data->nextnote[a] += 12;
                        }
                        else if(data->nextnote[a] <= 55){
                            data->nextnote[a] += 24;
                        }
                        else if(data->nextnote[a] >= 85){
                            data->nextnote[a] -= 12;
                        }
                        else if((int)(data->nextnote[a]) >= 90){
                            data->nextnote[a] -= 24;
                        }
        
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                fprintf(data->scorefile, "i800 0 3 0.7 %d 2 3 %d 0.6 0.5 %d 0.5 %d\n", data->nextnote[a], (data->blockBrghtnss[c] * 1000), (data->blockBrghtnss[c] * 1000), (data->blockColor[c] * 5));
                            }
                            else{
                                fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f %d . . %d . %d\n", data->notelength, data->nextnote[a], (data->notelength - 1), (data->blockBrghtnss[c] * 1000), (data->blockBrghtnss[c] * 1000), (data->blockColor[c] * 5));
                            }
                            lastBrghtnss = data->blockBrghtnss[c];
                            lastColor = data->blockColor[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                if(lastColor == data->blockColor[c]){
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f . . . . . .\n", data->notelength, data->nextnote[a], (data->notelength - 1));
                                }
                                else{
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f . . . . . <\n", data->notelength, data->nextnote[a], (data->notelength - 1));
                                }
                            }
                            else{
                                if(lastColor == data->blockBrghtnss[c]){
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f < . . < . .\n", data->notelength, data->nextnote[a], (data->notelength - 1));
                                }   
                                else{
                                    fprintf(data->scorefile, "i800 + %.2f 0.7 %d 2 %.2f < . . < . <\n", data->notelength, data->nextnote[a], (data->notelength - 1));   
                                }
                            }
                        }
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }

                    data->scorenum += 1;
                case 3:
                    data->printflag = 0;
                    c = 0;
                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 4.5;
                        //constraining the note numbers to their frequency ranges
                        if(data->nextnote[a] <= 35){
                            data->nextnote[a] += 12;
                        }
                        else if(data->nextnote[a] <= 25){
                            data->nextnote[a] += 24;
                        }
                        else if(data->nextnote[a] >= 60){
                            data->nextnote[a] -= 24;
                        }
                        else if((data->nextnote[a]) >= 71){
                            data->nextnote[a] -= 36;
                        }           
                        
                        
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the dark bass instrument
                                fprintf(data->scorefile, "i600 0 4 0.15 %d 1 3.8 %d\n", (data->midi_note), (data->blockBrghtnss[0] * 75));
                            }
                            else{
                                fprintf(data->scorefile, "i600 + %.2f . %d . %.2f %d\n", data->notelength, (data->nextnote[a]), (data->notelength - 0.5), (data->blockBrghtnss[0] * 75));
                            }    
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                fprintf(data->scorefile, "i600 + %.2f . %d . %.2f .\n", data->notelength, (data->nextnote[a]), (data->notelength - 0.5));
                            }   
                            else{
                                fprintf(data->scorefile, "i600 + %.2f . %d . %.2f <\n", data->notelength, (data->nextnote[a]), (data->notelength - 0.5));
                            }
                        }               
        
                        data->totalLength += (data->notelength + 1);
                        data->printflag++;
                    }           
                    data->scorenum += 1;
                case 4:

                    lengthtotal = 0;
                    count = 0;
                    data->printflag = 0;
                    c = 0;          
                    for(a = 0; a < data->length; a++){
                        data->notelength = (double)randnum() + 3;
                        rate = 2;
                        int pitch[3];
                        int z;
                        //fill up array indexes for the groups of pitches played by the rhythm instrument, then constrain them to their frequency range
                        for(z = 0; z < 3; z++){
                            pitch[z] = scales(data);
                            if(pitch[z] >= 65){
                                pitch[z] -= 12;
                            }
                            if(pitch[z] >= 70){
                                pitch[z] -= 24;
                            }
                        }
                        int rnd = ((rand() % 6) + 1);
                        count++;
                        //used to calculate the total amount of time taken up by the rhythm instrument, so that it doesn't play longer than the other instruments
                        lengthtotal += data->notelength;
                        //determine when to change the rate of the rhythm
                        if(count % rnd == 0){
                            rate = rate / 2;
                        }           
                        if((data->printflag % data->numBlocks) == 0){
                            if(data->printflag == 0){
                                //print the first line of the score for the rhythm instrument
                                fprintf(data->scorefile, "i400 0 %.2f 0.5 %d 0 0.1 1 %d\n", rate, (data->midi_note + 12), (data->blockBrghtnss[0] * 500));
                            }
                            else{
                                fprintf(data->scorefile, "i400 + %.2f 0.5 %d . . . %d\n", rate, (data->midi_note + 12), (data->blockBrghtnss[c] * 500));
                            }           
                            if(data->blockColor[c] >= 5){
                                rate = 2;
                            }
                            else{
                                rate = 1;
                            }               
                            lastBrghtnss = data->blockBrghtnss[c];
                            c++;    
                            if(c == 7){
                                c--;
                            }
                        }
                        else{
                            if(lastBrghtnss == data->blockBrghtnss[c]){
                                //additional lines for the rhythm instrument
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (data->nextnote[a] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[0] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[1] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . .\n", rate, (pitch[2] + 12));
                            }   
                            else{
                                //additional lines for the rhythm instrument
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (data->nextnote[a] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[0] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[1] + 12));
                                fprintf(data->scorefile, "i400 + %.2f . %d . . . <\n", rate, (pitch[2] + 12));
                            }
                        }           
                        data->totalLength += (data->notelength);
                        data->printflag++;
                       
                        if((data-> printflag == data->length) && (lengthtotal <= (data->totalLength - 30))){
                            a--;
                        }
                    }
                    data->scorenum += 1;
                case 5:
                    fprintf(data->scorefile, "i1000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i2000 0 %.2f 1 4\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i5000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i6000 0 %.2f\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "i7000 0 %.2f 2 3\n", ((data->totalLength / 3) + 10));
                    fprintf(data->scorefile, "e\n");
                    fprintf(data->scorefile, "</CsScore>\n</CsoundSynthesizer>\n");
                    break;
            }
            break;
    }
}

int runscore(boxdata *b){

	strcpy(b->commands, "csound -odac ");
	strcat(b->commands, b->filename);
	system(b->commands);

	return 0;
}

int cleanup(boxdata *c){

    if(c->saveFlag == 2){
        strcpy(c->commands, "rm ");
        strcat(c->commands, c->filename);
        system(c->commands);
    }

    printf("Deleting resources...\n");

	return 0;
}

int colorFinder(boxdata *data, int tflag){
    
    int i;
    double R;
    double G;
    double B;
    double all;
    char* color;
    char* brightness;
    int prevColor;
    
    color = malloc(250);
    brightness = malloc(250);

    data->numchanges = 0;

    for(i = 0; i < data->numBlocks; i++){
    	
    	if(tflag == 0){

    		R = (double)data->redAvg[i];
        	G = (double)data->greenAvg[i];
        	B = (double)data->blueAvg[i];
        	all = (R + G + B) / 3;

        	data->ravgs = R;
    		data->gavgs = G;
    		data->bavgs = B;
	
    		data->rsum += data->ravgs;
    		data->gsum += data->gavgs;
    		data->bsum += data->bavgs;
    	}

    	if(tflag == 1){

    		i = data->numBlocks - 1;

    		R = (double)(data->rsum / data->numBlocks);
    		G = (double)(data->gsum / data->numBlocks);
    		B = (double)(data->bsum / data->numBlocks);
            all = (R + G + B) / 3;
    	}

        if((fabs(R - G) < (all * 0.1) && (fabs(R - B) < (all * 0.1)))) {
            color = "Greyscale";
            data->midi_note = (int)((rand() % 4) + 35);
            if(tflag == 0){
            	data->blockColor[i] = 1; //1 for Greyscale
            }
            else{
                data->mainColor = 1;
                printf("Most Dominant Color: %s\n", color);
                printf("Color code: %d\n", data->mainColor);
            }
        }
        else if((R < G) && (R < B) && (fabs(G - B) < ((G + B) / 2) * 0.125)){
            color = "Cyan";
            data->midi_note = (int)((rand() % 4) + 60);
            if(tflag == 0){
            	data->blockColor[i] = 2; //2 for Cyan
            }
            else{
                data->mainColor = 2;
                printf("Most Dominant Color: %s\n", color);
                printf("Color code: %d\n", data->mainColor);
            }
        }
        else if((G < R) && (G < B) && (fabs(R - B) < ((R + B) / 2) * 0.125)){
            color = "Magenta";
			data->midi_note = (int)((rand() % 4) + 50);
            if(tflag == 0){
            	data->blockColor[i] = 3; //3 for Magenta
            }
            else{
                data->mainColor = 3;
                printf("Most Dominant Color: %s\n", color);
                printf("Color code: %d\n", data->mainColor);
            }
        }
        else if((B < R) && (B < G) && (fabs(R - G) < ((R + G) / 2) * 0.125)){
            color = "Yellow";
            data->midi_note = (int)((rand() % 4) + 70);
            if(tflag == 0){
            	data->blockColor[i] = 4; //4 for Yellow
            }
            else{
                data->mainColor = 4;
                printf("Most Dominant Color: %s\n", color);
                printf("Color code: %d\n", data->mainColor);
            }
        }
        else if((R > G) && (R > B)){
            color = "Red";
            data->midi_note = (int)((rand() % 4) + 45);
            if(tflag == 0){
            	data->blockColor[i] = 5; //5 for Red
            }
            else{
                data->mainColor = 5;
                printf("Most Dominant Color: %s\n", color);
                printf("Color code: %d\n", data->mainColor);
            }
        }
        else if((G > R) && (G > B)){
            color = "Green";
            data->midi_note = (int)((rand() % 4) + 55);
            if(tflag == 0){
            	data->blockColor[i] = 6; //6 for Green
            }
            else{
                data->mainColor = 6;
                printf("Most Dominant Color: %s\n", color);
                printf("Color code: %d\n", data->mainColor);
            }
        }
        else if((B > R) && (B > G)){
            color = "Blue";
            data->midi_note = (int)((rand() % 4) + 65);
            if(tflag == 0){
            	data->blockColor[i] = 7; //7 for Blue
            }
            else{
                data->mainColor = 7;
                printf("Most Dominant Color: %s\n", color);
                printf("Color code: %d\n", data->mainColor);
            }
        }

        if(all > 191){
            brightness = "Light";
            data->brightness = 7;
            if((R > 200) && (G > 200) && (B > 200)){
            	data->brightness = 3;
            }
            if(tflag == 0){
            	data->blockBrghtnss[i] = 3;
            }
            else{
                data->keychoice = 3;
                printf("Total Brightness level: %s\n", brightness);
            }
        }
        else if((all > 63) && (all < 191)){
            brightness = "Medium";
            data->brightness = 5;
            if((R > 77) && (G > 77) && (B > 77) && (R < 191) && (G < 191) && (B < 191)){
            	data->brightness = 2;
            }
            if(tflag == 0){
            	data->blockBrghtnss[i] = 2;
            }
            else{
                data->keychoice = 2;
                printf("Total Brightness level: %s\n", brightness);
            }
        }
        else{
            brightness = "Dark";
            data->brightness = 4;
            if((R < 50) && (G < 50) && (B < 50)){
            	data->brightness = 1;
            }
            if(tflag == 0){
            	data->blockBrghtnss[i] = 1;
            }
            else{
                data->keychoice = 1;
                printf("Total Brightness level: %s\n", brightness);
            }
        }
        
        if(tflag == 0){
            
            if(data->blockColor[i] != prevColor){
                data->numchanges += 1;  
            }
            
            prevColor = data->blockColor[i];

        	printf("Block %d - Most dominant color = %s\n", i, color);
        	printf("Block %d - Brightness = %s\n", i, brightness);
        }
    }

    return 0;
}

int blockAverages(FILE* color, boxdata *data){
    //scan the files for their values;
    rewind(color);

    unsigned int lastint = 0;
    unsigned int nextint = 0;
    unsigned int sum = 0;
    unsigned int average = 0;
    unsigned int numints = 0;
    int a;

    if(data->channel == 0){
        printf("Calculating Averages...\n");
    }
    data->channel += 1;
    data->numBlocks = 0;

    while(1){
        if(feof(color)){
            break;
        }

        for(a = 0; a < data->blockSize; a++){
            fscanf(color, "%d", &nextint);
            numints++;
            lastint += nextint;
            average = (lastint / numints);
        }

        if(data->channel == 1){
            data->redAvg[data->numBlocks] = average;
            printf("Red Channel - Block %d - Average: %d\n", data->numBlocks, data->redAvg[data->numBlocks]);
        }
        if(data->channel == 2){
            data->greenAvg[data->numBlocks] = average;
            printf("Green Channel - Block %d - Average: %d\n", data->numBlocks, data->greenAvg[data->numBlocks]);
        }
        if(data->channel == 3){
            data->blueAvg[data->numBlocks] = average;
            printf("Blue Channel - Block %d - Average: %d\n", data->numBlocks, data->blueAvg[data->numBlocks]);
        }

        data->numBlocks += 1;
    }

    return 0;
}

int imageDataProcess(boxdata *data){

    data->red = fopen("red.txt", "r");
    data->green = fopen("green.txt", "r");
    data->blue = fopen("blue.txt", "r");

    //get file sizes:
    unsigned int sizered = 0;
    unsigned int sizegreen = 0;
    unsigned int sizeblue = 0;
    unsigned int tmp;

    data->channel = 0;

    printf("Reading matrix files...\n");

    rewind(data->red);
    while(1){
        if(feof(data->red)){
            printf("Red: %d pixels...\n", sizered);
            break;
        }
        fscanf(data->red, "%d", &tmp);
        sizered++;
    }

    rewind(data->green);
    while(1){
        if(feof(data->green)){
            printf("Green: %d pixels...\n", sizegreen);
            break;
        }
        fscanf(data->green, "%d", &tmp);
        sizegreen++;
    }

    rewind(data->blue);
    while(1){
        if(feof(data->blue)){
            printf("Blue: %d pixels...\n", sizeblue);
            break;
        }
        fscanf(data->blue, "%d", &tmp);
        sizeblue++;
    }

    sleep(3);
    system("clear");

    //always enter channels in RGB order to avoid channel averages from being wrongly assigned
    data->blockSize = (sizered / 7);
    blockAverages(data->red, data);

    data->blockSize = (sizegreen / 7);
    blockAverages(data->green, data);

    data->blockSize = (sizeblue / 7);
    blockAverages(data->blue, data);

    colorFinder(data, 0);	//0 for blocks
    colorFinder(data, 1);	//1 for whole image

    //set the length of score to the number of pixel blocks made by the image
    data->length = pow((double)data->numBlocks, (double)2);

    fclose(data->red);
    fclose(data->green);
    fclose(data->blue);

    system("clear");

    return 0;
}
