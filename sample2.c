#include "F28x_Project.h"
float x=0, y=0, z;
void main(void){

InitSysCtrl();

DINT;
InitPieCtrl();
IER = 0x0000;
IFR = 0x0000;
InitPieVectTable();

while(1)
{
z=x+y;
}
}
