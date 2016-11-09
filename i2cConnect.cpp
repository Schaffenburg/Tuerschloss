//**************************************************************//
//  Name    : i2cConnect                                       //
//  Author  : Karsten Schlachter                                //
//  Date    : Oktober 2015 update September 16                  //
//  Version : 1.0                                               //
//  Notes   : uebertraegt i2c Kommandos an Tuer-Arduino         //
//  Web   : https://wiki.schaffenburg.org/Projekt:Tuerschloss   // 
//****************************************************************


//benoetigt gpio lib
//basiert auf beispiel

//**********************************
//compilieren mit
//g++ -lwiringPi  -o i2ctest i2ctest.cpp

#include <iostream>
#include <sstream>
#include <unistd.h>
#include "wiringPi.h"
#include "wiringPiI2C.h"

using namespace std;

const int address = 0x04; //i2c Adresse der Gegenseite 
int fd;


int writeChar(char value) {
return wiringPiI2CWrite(fd, value);
}

int writeString(char str[]){
	writeChar('\n');//cmd buffer im arduino clearen
	int index=0;
	while(str[index]!=0){
		writeChar(str[index++]);
		
		//100ms warten um bus nicht zu ueberlasten
		usleep(100000);
	}
	writeChar('\n');//cmd ausfuehren
}

int readNumber(void) {
return wiringPiI2CRead(fd);
}

int main(int argc, const char * argv[]) {
	unsigned int var;
	int number;

	fd = wiringPiI2CSetup(address); 
	char c;

	//ohne zusaetzl argumente testmodus starten
	if(argc==1){

		std::cout<<"Testmodus - '|' fuer newline"<<std::endl;

		std::cin>>c;
		while(c){
			if(c=='|') c='\n';
			writeChar(c);
			std::cout << "zu arduino: " << c << std::endl;
			number = readNumber();
			std::cout << "von arduino: " << number << std::endl;
			std::cin>>c;

			//100ms verzoegerung damit i2c das ganze frisst
			usleep(100000);
		}

	} else {
		//istringstream iss(argv[1]);
		//iss >> var;
		//writeChar(var);
		//cout << "";
		//cout<<"argument:"<<argv[1]<<endl;
		writeString((char*)argv[1]);
		usleep(1000);//warten, damit arduino reagieren kann
		number=readNumber();//antwort des arduinos auslesen
		cout << number;
	}
	return 0;
}
