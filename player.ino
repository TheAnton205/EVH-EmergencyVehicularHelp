
#include <SDHCI.h>
#include <Audio.h>

SDClass theSD;
AudioClass *theAudio;

File myFile;

bool ErrEnd = false;

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}

void setup()
{
  /* Initialize SD */
  while (!theSD.begin())
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

  // start audio system
  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* Set clock mode to normal */
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);

  /*
   * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect"
   * Search for MP3 decoder in "/mnt/sd0/BIN" directory
   */
  err_t err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_MONO);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Player0 initialize error\n");
      exit(1);
    }

  /* Open file placed on SD card */
  myFile = theSD.open("ask_help.mp3");

  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }
  printf("Open! 0x%08lx\n", (uint32_t)myFile);

  /* Send first frames to be decoded */
  err = theAudio->writeFrames(AudioClass::Player0, myFile);

  if ((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND))
    {
      printf("File Read Error! =%d\n",err);
      myFile.close();
      exit(1);
    }

  puts("Play!");

  /* Main volume set to -16.0 dB  MAXIMM is 120*/
  theAudio->setVolume(120);
  theAudio->startPlayer(AudioClass::Player0);
}

/**
 * @brief Play stream
 *
 * Send new frames to decode in a loop until file ends
 */
void endAudio() {
  theAudio->stopPlayer(AudioClass::Player0);
  myFile.close();
  theAudio->setReadyMode();
  theAudio->end();
  exit(1);
}
void play() { 
  while(1) {
    int err = theAudio->writeFrames(AudioClass::Player0, myFile);
    if (err == AUDIOLIB_ECODE_FILEEND)
      {
        printf("Main player File End!\n");
      }
  
    /* Show error code from player and stop */
    if (err)
      {
        printf("Main player error code: %d\n", err);
        return;
        //stop();
      }
  
    if (ErrEnd)
      {
        printf("Error End\n");
        return;
        //stop();
      }
  }
}
void loop()
{
  play();
  
  Serial.println("Playing!");
  delay(5000);
  
  myFile.close();
  myFile = theSD.open("ask_help.mp3");
  
  Serial.println("Restarting in 3, ");
  delay(1000);
  Serial.println("2, ");
  delay(1000);
  Serial.println("1, ");
  
  play();
  delay(5000);
  endAudio();
  Serial.println("Ended!");
  

  return;

}
