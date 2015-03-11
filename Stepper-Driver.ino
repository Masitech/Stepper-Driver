#include <SPI.h>
#include "/Users/saleembanatwala/Documents/Stepper-Driver/registers.h"

// Register Access

#define REGWRITE    0x00
#define REGREAD     0x80
#define SLEEP       12


//these are defined in registers.h
struct CTRL_Register 	G_CTRL_REG;
struct TORQUE_Register 	G_TORQUE_REG;
struct OFF_Register 	G_OFF_REG;
struct BLANK_Register	G_BLANK_REG;
struct DECAY_Register 	G_DECAY_REG;
struct STALL_Register 	G_STALL_REG;
struct DRIVE_Register 	G_DRIVE_REG;
struct STATUS_Register 	G_STATUS_REG;


void setup()
{
Serial.begin(115200);
pinMode(53,OUTPUT);  //SPI Slave Select
digitalWrite(53,LOW);
SPI.begin();
SPI.setClockDivider(SPI_CLOCK_DIV8);
pinMode(SLEEP, OUTPUT);
//pinMode(STEP, OUTPUT);
//pinMode(DIR, OUTPUT);
//pinMode(POT, INPUT);
digitalWrite(SLEEP, LOW);  //DRV8711 in Sleep Mode
//digitalWrite(STEP, LOW);  //toggle up and down to step
//digitalWrite(DIR, LOW);  //direction




// Set Default Register Settings

	// CTRL Register
	G_CTRL_REG.Address 	= 0x00;
	G_CTRL_REG.DTIME 	= 0x03;  //850ns
	G_CTRL_REG.ISGAIN 	= 0x01;  //Gain of 10
	G_CTRL_REG.EXSTALL 	= 0x00;  //Internal Stall Detect
	G_CTRL_REG.MODE 	= 0x03;  // 1/8 Step
	G_CTRL_REG.RSTEP 	= 0x00;  //No Action
	G_CTRL_REG.RDIR 	= 0x00;  //Direction set by DIR Pin
	G_CTRL_REG.ENBL 	= 0x01;  //enable motor
        //1000 11 01 0 0011 0 0 1

	/// TORQUE Register
	G_TORQUE_REG.Address = 0x01;
	G_TORQUE_REG.SIMPLTH = 0x00;  //50uS Back EMF Sample Threshold
	G_TORQUE_REG.TORQUE  = 0x46;  //with ISGAIN set to 10 this gives about 1.5 AMP Current Chopper
        //1000 0 000  01000110
        
	// OFF Register
	G_OFF_REG.Address 	= 0x02;
	G_OFF_REG.PWMMODE 	= 0x00;  //Internal Indexer
	G_OFF_REG.TOFF 		= 0x30;  //Default
        //1000 000 0 00110000

	// BLANK Register
	G_BLANK_REG.Address = 0x03;
	G_BLANK_REG.ABT 	= 0x01;  //enable adaptive blanking time
	G_BLANK_REG.TBLANK 	= 0x08;  //no idea what this should be but the
        //1000 000 1 00001000            //user guide shows it set to this

	// DECAY Register.
	G_DECAY_REG.Address = 0x04;
	G_DECAY_REG.DECMOD  = 0x03;  //mixed decay
	G_DECAY_REG.TDECAY 	= 0x10;  //default
        //1000001100010000

	// STALL Register
	G_STALL_REG.Address = 0x05;
	G_STALL_REG.VDIV 	= 0x03;  //Back EMF is divided by 4
	G_STALL_REG.SDCNT 	= 0x03;  //stalln asserted after 8 steps
	G_STALL_REG.SDTHR 	= 0x40;  //default
        //1000111101000000

	// DRIVE Register
	G_DRIVE_REG.Address = 0x06;
	G_DRIVE_REG.IDRIVEP = 0x00;  //High Side 50mA peak (source)
	G_DRIVE_REG.IDRIVEN = 0x00;  //Low Side 100mA peak (sink)
	G_DRIVE_REG.TDRIVEP = 0x01;  //High Side gate drive 500nS
	G_DRIVE_REG.TDRIVEN = 0x01;  //Low Side Gate Drive 500nS
	G_DRIVE_REG.OCPDEG = 0x01;  //OCP Deglitch Time 2uS
	G_DRIVE_REG.OCPTH = 0x01;  //OCP Threshold 500mV
        //1000000001010101

	// STATUS Register
	G_STATUS_REG.Address = 0x07;
	G_STATUS_REG.STDLAT  = 0x00;
	G_STATUS_REG.STD     = 0x00;  
	G_STATUS_REG.UVLO    = 0x00;  
	G_STATUS_REG.BPDF    = 0x00;  
	G_STATUS_REG.APDF    = 0x00;
	G_STATUS_REG.BOCP    = 0x00;
	G_STATUS_REG.AOCP    = 0x00;
	G_STATUS_REG.OTS     = 0x00;
        
        digitalWrite(SLEEP, HIGH);
        WriteAllRegisters();
        ReadAllRegisters();
}


void loop()
{
  
  //Serial.println(analogRead(POT));
  if(Serial.available())
    {
      char c = Serial.read();
      if(c == 'd')
      {
        G_CTRL_REG.ENBL 	= 0x00;  //disable motor
        WriteAllRegisters();
        Serial.println("Motor Disabled");
      }
      
      if(c == 'e')
      {
        G_CTRL_REG.ENBL 	= 0x01;  //enable motor
        WriteAllRegisters();
        Serial.println("Motor Enabled");
      }
      if(c=='r') 
      {      
        ReadAllRegisters();
        //Serial.println(analogRead(POT));
      }
      
//      if(c=='u')
//      {
//        G_STATUS_REG.UVLO = 0x00;  //clear undervoltage lock out
//        WriteAllRegisters();
//      }
      
    }
        
  //for(int i = 0; i < 1600; i++)
  //{
//    digitalWrite(STEP, HIGH);
//    digitalWrite(STEP, LOW);
//    delay(analogRead(POT));
  //}
  
  //Serial.println("Next");
}

void ReadAllRegisters()
{
    unsigned char dataHi = 0x00;
    const unsigned char dataLo = 0x00;
    unsigned int readData = 0x00;

    // Read CTRL Register
    dataHi = REGREAD | (G_CTRL_REG.Address << 4);
    readData = SPI_DRV8711_ReadWrite(dataHi, dataLo);
    Serial.println(readData, BIN);
    G_CTRL_REG.DTIME        = ((readData >> 10) & 0x0003);
    G_CTRL_REG.ISGAIN       = ((readData >> 8) & 0x0003);
    G_CTRL_REG.EXSTALL      = ((readData >> 7) & 0x0001);
    G_CTRL_REG.MODE         = ((readData >> 3) & 0x000F);
    G_CTRL_REG.RSTEP        = ((readData >> 2) & 0x0001);
    G_CTRL_REG.RDIR         = ((readData >> 1) & 0x0001);
    G_CTRL_REG.ENBL         = ((readData >> 0) & 0x0001);
    
    Serial.println("Control Register");
    Serial.print("DTIME = ");
    Serial.println(G_CTRL_REG.DTIME, BIN);
    Serial.print("ISGAIN = ");
    Serial.println(G_CTRL_REG.ISGAIN, BIN);
    Serial.print("EXSTALL = ");
    Serial.println(G_CTRL_REG.EXSTALL, BIN);
    Serial.print("MODE = ");
    Serial.println(G_CTRL_REG.MODE, BIN);
    Serial.print("RSTEP = ");
    Serial.println(G_CTRL_REG.RSTEP, BIN);
    Serial.print("RDIR = ");
    Serial.println(G_CTRL_REG.RDIR, BIN);
    Serial.print("ENBL = ");
    Serial.println(G_CTRL_REG.ENBL, BIN);

    // Read TORQUE Register
    dataHi = REGREAD | (G_TORQUE_REG.Address << 4);
    readData = SPI_DRV8711_ReadWrite(dataHi, dataLo);
    Serial.println(readData, BIN);
    G_TORQUE_REG.SIMPLTH    = ((readData >> 8) & 0x0007);
    G_TORQUE_REG.TORQUE     = ((readData >> 0) & 0x00FF);
    Serial.print("TORQUE = ");
    Serial.println(G_TORQUE_REG.TORQUE, HEX);

    // Read OFF Register
    dataHi = REGREAD | (G_OFF_REG.Address << 4);
    readData = SPI_DRV8711_ReadWrite(dataHi, dataLo);
    Serial.println(readData, BIN);
    G_OFF_REG.PWMMODE       = ((readData >> 8) & 0x0001);
    G_OFF_REG.TOFF          = ((readData >> 0) & 0x00FF);

    // Read BLANK Register
    dataHi = REGREAD | (G_BLANK_REG.Address << 4);
    readData = SPI_DRV8711_ReadWrite(dataHi, dataLo);
    Serial.println(readData, BIN);
    G_BLANK_REG.ABT         = ((readData >> 8) & 0x0001);
    G_BLANK_REG.TBLANK      = ((readData >> 0) & 0x00FF);

    // Read DECAY Register
    dataHi = REGREAD | (G_DECAY_REG.Address << 4);
    readData = SPI_DRV8711_ReadWrite(dataHi, dataLo);
    Serial.println(readData, BIN);
    G_DECAY_REG.DECMOD      = ((readData >> 8) & 0x0007);
    G_DECAY_REG.TDECAY      = ((readData >> 0) & 0x00FF);

    // Read STALL Register
    dataHi = REGREAD | (G_STALL_REG.Address << 4);
    readData = SPI_DRV8711_ReadWrite(dataHi, dataLo);
    Serial.println(readData, BIN);
    G_STALL_REG.VDIV        = ((readData >> 10) & 0x0003);
    G_STALL_REG.SDCNT       = ((readData >> 8) & 0x0003);
    G_STALL_REG.SDTHR       = ((readData >> 0) & 0x00FF);

    // Read DRIVE Register
    dataHi = REGREAD | (G_DRIVE_REG.Address << 4);
    readData = SPI_DRV8711_ReadWrite(dataHi, dataLo);
    Serial.println(readData, BIN);
    G_DRIVE_REG.IDRIVEP     = ((readData >> 10) & 0x0003);
    G_DRIVE_REG.IDRIVEN     = ((readData >> 8) & 0x0003);
    G_DRIVE_REG.TDRIVEP     = ((readData >> 6) & 0x0003);
    G_DRIVE_REG.TDRIVEN     = ((readData >> 4) & 0x0003);
    G_DRIVE_REG.OCPDEG      = ((readData >> 2) & 0x0003);
    G_DRIVE_REG.OCPTH       = ((readData >> 0) & 0x0003);

    // Read STATUS Register
    dataHi = REGREAD | (G_STATUS_REG.Address << 4);
    readData = SPI_DRV8711_ReadWrite(dataHi, dataLo);
    Serial.println(readData, BIN);
    G_STATUS_REG.STDLAT     = ((readData >> 7) & 0x0001);
    G_STATUS_REG.STD        = ((readData >> 6) & 0x0001);
    G_STATUS_REG.UVLO       = ((readData >> 5) & 0x0001);
    G_STATUS_REG.BPDF       = ((readData >> 4) & 0x0001);
    G_STATUS_REG.APDF       = ((readData >> 3) & 0x0001);
    G_STATUS_REG.BOCP       = ((readData >> 2) & 0x0001);
    G_STATUS_REG.AOCP       = ((readData >> 1) & 0x0001);
    G_STATUS_REG.OTS        = ((readData >> 0) & 0x0001);
}

void WriteAllRegisters()
{
    unsigned char dataHi = 0x00;
    unsigned char dataLo = 0x00;

    // Write CTRL Register
    dataHi = REGWRITE | (G_CTRL_REG.Address << 4) | (G_CTRL_REG.DTIME << 2) | (G_CTRL_REG.ISGAIN);
    dataLo = (G_CTRL_REG.EXSTALL << 7) | (G_CTRL_REG.MODE << 3) | (G_CTRL_REG.RSTEP << 2) | (G_CTRL_REG.RDIR << 1) | (G_CTRL_REG.ENBL);
    SPI_DRV8711_ReadWrite(dataHi, dataLo);

    // Write TORQUE Register
    dataHi = REGWRITE | (G_TORQUE_REG.Address << 4) | (G_TORQUE_REG.SIMPLTH);
    dataLo = G_TORQUE_REG.TORQUE;
    SPI_DRV8711_ReadWrite(dataHi, dataLo);

    // Write OFF Register
    dataHi = REGWRITE | (G_OFF_REG.Address << 4) | (G_OFF_REG.PWMMODE);
    dataLo = G_OFF_REG.TOFF;
    SPI_DRV8711_ReadWrite(dataHi, dataLo);

    // Write BLANK Register
    dataHi = REGWRITE | (G_BLANK_REG.Address << 4) | (G_BLANK_REG.ABT);
    dataLo = G_BLANK_REG.TBLANK;
    SPI_DRV8711_ReadWrite(dataHi, dataLo);

    // Write DECAY Register
    dataHi = REGWRITE | (G_DECAY_REG.Address << 4) | (G_DECAY_REG.DECMOD);
    dataLo = G_DECAY_REG.TDECAY;
    SPI_DRV8711_ReadWrite(dataHi, dataLo);

    // Write STALL Register
    dataHi = REGWRITE | (G_STALL_REG.Address << 4) | (G_STALL_REG.VDIV << 2) | (G_STALL_REG.SDCNT);
    dataLo = G_STALL_REG.SDTHR;
    SPI_DRV8711_ReadWrite(dataHi, dataLo);

    // Write DRIVE Register
    dataHi = REGWRITE | (G_DRIVE_REG.Address << 4) | (G_DRIVE_REG.IDRIVEP << 2) | (G_DRIVE_REG.IDRIVEN);
    dataLo = (G_DRIVE_REG.TDRIVEP << 6) | (G_DRIVE_REG.TDRIVEN << 4) | (G_DRIVE_REG.OCPDEG << 2) | (G_DRIVE_REG.OCPTH);
    SPI_DRV8711_ReadWrite(dataHi, dataLo);

    // Write STATUS Register
    dataHi = REGWRITE | (G_STATUS_REG.Address << 4);
    dataLo = (G_STATUS_REG.STDLAT << 7) | (G_STATUS_REG.STD << 6) | (G_STATUS_REG.UVLO << 5) | (G_STATUS_REG.BPDF << 4) | (G_STATUS_REG.APDF << 3) | (G_STATUS_REG.BOCP << 2) | (G_STATUS_REG.AOCP << 1) | (G_STATUS_REG.OTS);
    SPI_DRV8711_ReadWrite(dataHi, dataLo);
}

unsigned int SPI_DRV8711_ReadWrite(unsigned char dataHi, unsigned char dataLo)
{
	unsigned int readData = 0;

	digitalWrite(53, HIGH);
	
	readData |= (SPI.transfer(dataHi) << 8);
	readData |= SPI.transfer(dataLo);

	digitalWrite(53, LOW);

	return readData;
}
