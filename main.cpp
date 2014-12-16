#include "mbed.h"
#include "lua.hpp"  //the extern C version of headers...
#include "SDFileSystem.h"
#include "FXOS8700Q.h"

SDFileSystem sd(PTE3, PTE1, PTE2, PTE4, "sd"); // MOSI, MISO, SCK, CS
Serial pc(USBTX, USBRX);
FILE *fp;
char buffer[1024];

//define some led's
DigitalOut red  (LED_RED);
DigitalOut green(LED_GREEN);
DigitalOut blue (LED_BLUE);

Ticker flipper;
 
//FXOS8700Q acc( A4, A5, FXOS8700CQ_SLAVE_ADDR0); // Proper Ports and I2C address for Freescale Multi Axis shield
//FXOS8700Q mag( A4, A5, FXOS8700CQ_SLAVE_ADDR0); // Proper Ports and I2C address for Freescale Multi Axis shield
FXOS8700Q_acc acc( PTE25, PTE24, FXOS8700CQ_SLAVE_ADDR1); // Proper Ports and I2C Address for K64F Freedom board
FXOS8700Q_mag mag( PTE25, PTE24, FXOS8700CQ_SLAVE_ADDR1); // Proper Ports and I2C Address for K64F Freedom board
 

 
MotionSensorDataUnits mag_data;
MotionSensorDataUnits acc_data;
 
MotionSensorDataCounts mag_raw;
MotionSensorDataCounts acc_raw;

void flip() {
    green = !green;
}

void SetMyRTCTime(void)
{
	// get the current time from the terminal
	struct tm t;
	printf("\r\nEnter current date and time:\r\n");
	printf("YYYY MM DD HH MM SS[enter]\r\n");
	scanf("%d %d %d %d %d %d", &t.tm_year, &t.tm_mon, &t.tm_mday
                         , &t.tm_hour, &t.tm_min, &t.tm_sec);

	// adjust for tm structure required values
	t.tm_year = t.tm_year - 1900;
	t.tm_mon = t.tm_mon - 1;

	// set the time
	set_time(mktime(&t));

    time_t seconds = time(NULL);
    printf("\r\nTime as a basic string = %s\r\n\r\n", ctime(&seconds));
}
 
void Blink_Red(int numtimes)
{
	blue = 1;
	green = 1;
    while(numtimes > 0)
    {
        red = (numtimes&1);
        numtimes--;
        wait(1.0);
    }
    red = 1;
} 
 
void Blink_Blue(int numtimes)
{
	red = 1;
	green = 1;
    while(numtimes > 0)
    {
        blue = (numtimes&1);
        numtimes--;
        wait(0.3);
    }
    blue = 1;
} 
 
void Blink_Green(int numtimes)
{
	blue = 1;
	red = 1;
    while(numtimes > 0)
    {
        green = (numtimes&1);
        numtimes--;
        wait(0.1);
    }
    green = 1;
} 
 
 
void motiontest(int times) {
float faX, faY, faZ;
float fmX, fmY, fmZ;
int16_t raX, raY, raZ;
int16_t rmX, rmY, rmZ;
acc.enable();
printf("\r\n\nFXOS8700Q Who Am I= %X\r\n", acc.whoAmI());
    while (times > 0) {
        acc.getAxis(acc_data);
        mag.getAxis(mag_data);
        printf("FXOS8700Q ACC: X=%1.4f Y=%1.4f Z=%1.4f  ", acc_data.x, acc_data.y, acc_data.z);
        printf("    MAG: X=%4.1f Y=%4.1f Z=%4.1f\r\n", mag_data.x, mag_data.y, mag_data.z);
        acc.getX(&faX);
        acc.getY(&faY);
        acc.getZ(&faZ);
        mag.getX(&fmX);
        mag.getY(&fmY);
        mag.getZ(&fmZ);
        printf("FXOS8700Q ACC: X=%1.4f Y=%1.4f Z=%1.4f  ", faX, faY, faZ);
        printf("    MAG: X=%4.1f Y=%4.1f Z=%4.1f\r\n", fmX, fmY, fmZ);
        acc.getAxis(acc_raw);
        mag.getAxis(mag_raw);
        printf("FXOS8700Q ACC: X=%d Y=%d Z=%d  ", acc_raw.x, acc_raw.y, acc_raw.z);
        printf("    MAG: X=%d Y=%d Z=%d\r\n", mag_raw.x, mag_raw.y, mag_raw.z);
        acc.getX(&raX);
        acc.getY(&raY);
        acc.getZ(&raZ);
        mag.getX(&rmX);
        mag.getY(&rmY);
        mag.getZ(&rmZ);                
        printf("FXOS8700Q ACC: X=%d Y=%d Z=%d  ", raX, raY, raZ);
        printf("    MAG: X=%d Y=%d Z=%d\r\n\n", rmX, rmY, rmZ);    
        wait(1.0);
        times--;
    }
    Blink_Blue(20);
}





extern "C" void do_lua_getstring(char * b);

void do_lua_getstring(char *b)
{
	int ptr = 0;
	b[ptr] = 0;
	char mychar = 0;
	while((mychar != 0x0d) && (ptr < LUA_MAXINPUT-3))
	{
		mychar = pc.getc();
		if(mychar == 0x7f)//handle delete...
		{
			if(ptr>0)
			{
				ptr--;
				pc.putc(0x08);
				pc.putc(' '); //put a space to clear screen
				pc.putc(0x08); //advance back
			}
		}
		if((mychar >= ' ') && (mychar != 0x7f) )
		{
			b[ptr++]=mychar;
			pc.putc(mychar);
		}
	}
	pc.putc('\r');
	pc.putc('\n');
	b[ptr++]='\r';
	b[ptr++]= '\n';
	b[ptr] = 0;
	//pc.printf("\r\ndebug::: %s\r\n",b);
	return;
}

int do_lua (int argc,char **argv) {
  int status, result;
  lua_State *L = luaL_newstate();
  if (L == NULL) {
    pc.printf("\r\ncannot create state: not enough memory\r\n");
    return EXIT_FAILURE;
  }

  lua_pushcfunction(L, &pmain);
  lua_pushinteger(L, argc);
  lua_pushlightuserdata(L, argv);
  status = lua_pcall(L, 2, 1, 0);
  result = lua_toboolean(L, -1);
  //finalreport(L, status);
  lua_close(L);
  return (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

void bail(lua_State *L, char *msg){
	fprintf(stderr, "\nFATAL ERROR:\n  %s: %s\n\n",
		msg, lua_tostring(L, -1));
	exit(1);
}

int main()
{
	pc.baud(115200);
	red = 1;
	blue = 1;
	green = 1;
    SetMyRTCTime();

    motiontest(20);

    //will now store some acceleration data...
    int numreadings = 30;
    int ctr = 1;
    fp = fopen("/sd/acceldata.txt", "r");
    if (fp != NULL) {
        fclose(fp);
        remove("/sd/acceldata.txt");
        pc.printf("\r\nRemoved acceldata.txt \r\n");
    }

    printf("\r\nWriting accel data to the sd card -takes some time...\r\n");
    fp = fopen("/sd/acceldata.txt", "w");
    if (fp == NULL) {
        pc.printf("\r\nUnable to write the file \r\n");
    } else {
        while(numreadings > 0)
        {
            time_t seconds = time(NULL);
            //printf("\r\nTime as a basic string = %s\r\n\r\n", ctime(&seconds));
            acc.getAxis(acc_data);
            mag.getAxis(mag_data);
            fprintf(fp,"\r\nCounter: %d Time: %s\r\n",ctr,ctime(&seconds));
            fprintf(fp,"  FXOS8700Q ACC: X=%1.4f Y=%1.4f Z=%1.4f  \r\n", acc_data.x, acc_data.y, acc_data.z);
            fprintf(fp,"    MAG: X=%4.1f Y=%4.1f Z=%4.1f\r\n", mag_data.x, mag_data.y, mag_data.z);
            numreadings--;
            ctr++;
            //wait(0.5);
            Blink_Green(10);
        }
        fclose(fp);
        pc.printf("\r\nAccel data file successfully written! \r\n");
    }


	int rval;

    int i = 0;
    pc.printf("\r\nHello Lua!\r\n");


    lua_State *L;

    L = luaL_newstate();                        /* Create Lua state variable */
    luaL_openlibs(L);                           /* Load Lua libraries */
    
    printf("\r\n    ----Calling Lua script on sdcard...\r\n");
    dofile(L,"/sd/helloscript.lua");
    flipper.attach(&flip, 4.0); // the address of the function to be attached (flip) and the interval (4 seconds)

    rval = simple_lua(L);

    lua_close(L);                               /* Clean up, free the Lua state var */

    pc.printf("\r\nLua ret val: %d\r\n",rval);
    while (true) {
        wait(1.0f); // wait a small period of time
        if((i%10)==0)
        {
        	pc.printf("\r\n");
        }
        pc.printf("%d \n", i); // print the value of variable i
        i++; // increment the variable
        //myled = !myled; // toggle a led
        Blink_Blue(100);
    }
    Blink_Green(10000);
    for(;;);
}
