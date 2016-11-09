#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>


using namespace std;
//#include <stdlib.h>
//Written by: Spencer Pullins
//A program that takes the width and length and creates a binary matrix file

const int NUMBER_RANDOM = 1000;

int main(int argc, char *argv[]){


  if(argc != 3){
	//printf("Wrong number of args!");
        cout << "\nWrong number of args!\n";
	return 1;
  }

 // FILE *fp;
  //fp = fopen("matrix.in", "w+");
  srand(time(NULL));

  ofstream myfile;
  myfile.open("matrix.in");

  string temp1 = string(argv[1]);
  string temp2 = string(argv[2]); 
  string temp = temp1 + "\n";  // + " " + temp2 + "\n"; 
  myfile << temp; //argv[1] + " " + argv[2] + "\n";

  int width = 2*atoi(argv[1]);
  int length = atoi(argv[2]);
  //char *firstLine[]  = argv[1] + " " + argv[2] + "\n";

  //fprintf(fp, firstLine);
  //fprintf(fp,argv[1]);
  //fprintf(fp, " ");
  //fprintf(fp, argv[2]);
  //fprintf(fp, + "\n");

  for(int i = 0; i < length; i++){
	//move down one line
  	for(int i = 0; i < width; i++){
		//Add characters to line
		//fprintf(fp, rand() % 2 );
		//string s = to_string(rand()%2);
		//fp << rand() % 2;
		myfile << rand() % NUMBER_RANDOM;
		if(i != width-1){
			myfile << " ";
		}
	}
	//fprintf(fp, "\n");
	myfile << "\n";

  }
 //fclose(fp)
 myfile.close();
 return 0;
}