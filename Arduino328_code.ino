#include <SPI.h>
#include <RAS.h> // Rugged Audio Shield library

#include <MIDI.h> //--------------------------------------------------****MIDISHIELD****

//#define FILEX "RECFILE.WAV"
#define FILEPRESIZE 50
#define RECPIN 3
#define PLAYPIN 2
RAS RAS;
RAS_State_t RAS_state;
unsigned int idlecounter = 0;
char FILEX[10] = "REC00.WAV";
char FILEPREV[10] = "REC00.WAV";

boolean recpress = false;
boolean recmode = false;
boolean playpress = false;
boolean playmode = false;

void FileInc () //to increment FILEX, assuming initial value is "REC00.WAV"
{
  FILEPREV[4] = FILEX[4];
  FILEPREV[3] = FILEX[3];
  if (FILEX[4] <= 56) //56 is ascii code for 8. 57 for 9, 48 for 0.
  {
    //Serial.println(FILEX[4], DEC);
    FILEX[4]++;
    //Serial.println(FILEX[4], DEC);
  }
  else if (FILEX[3] <= 56) //FILEX[4] is already 57 i.e. 9
  {
    FILEX[4] = 48;
    FILEX[3]++;
  }
  else //FILEX[3} is also already 57 ie. 9
  {
    //reset to 00
    FILEX[4] = 48;
    FILEX[3] = 48;
  }
  //Serial.print("New & old files: "); Serial.print(FILEX); Serial.print("  "); Serial.println(FILEPREV);
}

void setup(void) {
  MIDI.begin(MIDI_CHANNEL_OMNI); //--------------------------------------------------****MIDISHIELD****
  
  pinMode(RECPIN, INPUT); digitalWrite(RECPIN, HIGH);
  pinMode(PLAYPIN, INPUT); digitalWrite(PLAYPIN, HIGH);
  //Serial.begin(9600); //conflicts with midi baud setting of 31250, so remove in final renditions.
  
  
  RAS.begin(); RAS.WaitForIdle();
  RAS.SetInputGainLine(INPUT_GAIN_4X); RAS.WaitForIdle(); //try 4x
  RAS.SetInputGainMic(INPUT_GAIN_4X); RAS.WaitForIdle();
  RAS.OutputEnable(); RAS.WaitForIdle();
  RAS.OutputBassBoostDisable(); RAS.WaitForIdle();
  RAS.OutputGainBoostEnable(); RAS.WaitForIdle();
  RAS.OutputVolumeSet(31); RAS.WaitForIdle(); //maxvolume
  
  //delay(5000);
  
  RAS.InitSD(); RAS.WaitForIdle(); 
  RAS.PresizeFile(FILEX, FILEPRESIZE); RAS.WaitForIdle();
  
  //RAS.EraseFilesystem(); RAS.WaitForIdle();
  //RAS.InitSD(SPI_RATE_4MHz); RAS.WaitForIdle();
 
  //RAS.PlayWAV("SOMEFILE.WAV"); RAS.WaitForIdle();
  
  //RAS.AudioEffect(EFFECT_NONE, 32000, SOURCE_STEREO, SOURCE_MIC);
  
   //RAS.PresizeFile(FILEX, FILEPRESIZE); RAS.WaitForIdle();
   
   //RAS.RecordWAV(32000, SOURCE_MONO, SOURCE_LINE, FILEX); //30464
   //delay(20000); RAS.Stop(); RAS.WaitForIdle();
   
}

void loop(void) {
  MIDI.read(); //--------------------------------------------------****MIDISHIELD****
  //FileInc();
  
  /*
  RAS.ReadInfo(); RAS_state = RAS.GetState();
  if (RAS_state == STATE_IDLE)
    Serial.println(1);
  else if (RAS_state == STATE_BUSY)
    Serial.println(2);
  else if (RAS_state == STATE_RECORDING_TO_SD)
    Serial.println(3);
  else if (RAS_state == STATE_PLAYING_FROM_SD)
    Serial.println(4);
  else if (RAS_state == STATE_PLAYING_FROM_SPI)
    Serial.println(5);
  else if (RAS_state == STATE_PASS_THROUGH)
    Serial.println(6);
  */
  
  //recording
  if (playmode == false)
  {
    if (digitalRead(RECPIN) == LOW)
    {
      recpress = true;
    }
    else if (recpress == true && digitalRead(RECPIN) == HIGH)
    {
      if (recmode == false)
      {
        recmode = true;
        recpress = false;
        //record
        //Serial.print("-recstart  "); Serial.println(FILEX);
        //RAS.PresizeFile(FILEX, FILEPRESIZE); RAS.WaitForIdle();
        RAS.RecordWAV(32000, SOURCE_MONO, SOURCE_LINE, FILEX); //30464
      }
      else //recmode==true implied
      {
        recmode = false;
        recpress = false;
        //stoprecord
        //Serial.print("--recstop  "); Serial.println(FILEX);
        RAS.Stop(); RAS.WaitForIdle();
        FileInc();
        //Serial.println("///procx");
        RAS.PresizeFile(FILEX, FILEPRESIZE); RAS.WaitForIdle();
        //Serial.println("///procxend");
      }
    }
  }  
  
  //playing
  if (recmode == false)
  {
    if (digitalRead(PLAYPIN) == LOW)
    {
      playpress = true;
    }
    else if (playpress == true && digitalRead(PLAYPIN) == HIGH)
    {
      if (playmode == false)
      {
        playmode = true;
        playpress = false;
        idlecounter = 0;
        //play
        //Serial.print("*playstart  "); Serial.println(FILEPREV);
        RAS.PlayWAV(FILEPREV);
      }
      else //playmode==true implied
      {
        playmode = false;
        playpress = false;
        idlecounter = 0;
        //stopplay
        //Serial.print("**playstop  "); Serial.println(FILEPREV);
        RAS.Stop(); RAS.WaitForIdle();
      }
    }
    else if (playpress == false && playmode == true) //detect end of played track and loop
    {
      RAS.ReadInfo(); RAS_state = RAS.GetState();
      
      if (idlecounter >= 20) //check twenty times for idle state (to guarantee it really IS idle)
      {
        playmode = true;
        playpress = false;
        idlecounter = 0;
        //play
        //Serial.print("###loop  "); Serial.println(FILEPREV);
        RAS.PlayWAV(FILEPREV);
      }
      else if (RAS_state == STATE_IDLE)
      {
        idlecounter++;
      }
    }
  }
  
  /*
  delay(1000);
  RAS.PlayWAV(FILEX); RAS.WaitForIdle();
  */
}
