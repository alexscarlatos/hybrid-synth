  #include <Wire.h>  
  #include <LiquidCrystal_I2C.h> // Using version 1.2.1
  #include <MIDI.h>  // Add Midi Library

  #define OFF_MODE 0
  #define PRESET_MODE 1
  #define WAVE_TYPE_MODE 2
  #define ATTACK_MODE 3
  #define DECAY_MODE 4
  #define SUSTAIN_MODE 5
  #define RELEASE_MODE 6

  #define SINE_SETTING 0
  #define SQUARE_SETTING 1
  #define TRI_SETTING 2
  #define SAW_SETTING 3
  
  #define ENV_ATTACK 0
  #define ENV_DECAY 1
  #define ENV_SUSTAIN 2
  #define ENV_RELEASE 3
  #define ENV_OFF 4

  #define NUM_PRESETS 5
  
  // ------ LED VARIABLES ------
  const int ledPin = 13;
  const int extLed1R = 2;
  const int extLed1B = 3;
  const int extLed1G = 4;
  const int extLed2R = 5;
  const int extLed2B = 6;
  const int extLed2G = 7;
  int led1R = 100, led1G = 0, led1B = 200, led2R = 50, led2G = 100, led2B = 150;

  // ------ POTENTIOMETER VARIABLES ------
  const int pot1Pin = 1;
  const int pot2Pin = 0;
  boolean usingPots = true;
  boolean usingLCD = true;
  boolean usingDAC = true;
  
  // ------ INTERACTION VARIABLES ------
  int potVal1 = 0, potVal2 = 0;
  float maxPotVal = 1023;
  
  typedef struct {
    int numValues;
    int* values;
    String modeName;
  } mode;
  
  volatile byte currentMode = OFF_MODE;
  int currentSetting = 0;
  int numModes = 7;
  mode modes[7];

  String waveTypeNames[4];
  String sustainLevelNames[10];
  // String presetNames[4];

  // Maps preset index to array of mode values
  int currentPreset = 0;
  int presets[NUM_PRESETS][5] = {
    {SQUARE_SETTING, 900, 900, 20, 900},
    {SINE_SETTING, 100, 100, 20, 300},
    {SQUARE_SETTING, 0, 0, 20, 100},
    {SQUARE_SETTING, 900, 0, 2, 900},
    {TRI_SETTING, 0, 300, 4, 500}
  };

  // LCD constructor - address is 0x3F
  // Also based on YWRobot LCM1602 IIC V1
  LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 
  
  // ------ DSP VARIABLES ------
  int currentWaveType = SINE_SETTING;
  int currentFrequency = 440;
  
  // Sine Wave Stuff
  const int precalculatedSteps = 5000;
  byte sine[] = {127,127,127,127,127,127,127,128,128,128,128,128,128,129,129,129,129,129,129,130,130,130,130,130,130,130,131,131,131,131,131,131,132,132,132,132,132,132,133,133,133,133,133,133,134,134,134,134,134,134,134,135,135,135,135,135,135,136,136,136,136,136,136,137,137,137,137,137,137,137,138,138,138,138,138,138,139,139,139,139,139,139,140,140,140,140,140,140,141,141,141,141,141,141,141,142,142,142,142,142,142,143,143,143,143,143,143,144,144,144,144,144,144,144,145,145,145,145,145,145,146,146,146,146,146,146,147,147,147,147,147,147,147,148,148,148,148,148,148,149,149,149,149,149,149,150,150,150,150,150,150,150,151,151,151,151,151,151,152,152,152,152,152,152,152,153,153,153,153,153,153,154,154,154,154,154,154,155,155,155,155,155,155,155,156,156,156,156,156,156,157,157,157,157,157,157,157,158,158,158,158,158,158,159,159,159,159,159,159,159,160,160,160,160,160,160,161,161,161,161,161,161,161,162,162,162,162,162,162,163,163,163,163,163,163,163,164,164,164,164,164,164,165,165,165,165,165,165,165,166,166,166,166,166,166,167,167,167,167,167,167,167,168,168,168,168,168,168,168,169,169,169,169,169,169,170,170,170,170,170,170,170,171,171,171,171,171,171,171,172,172,172,172,172,172,173,173,173,173,173,173,173,174,174,174,174,174,174,174,175,175,175,175,175,175,175,176,176,176,176,176,176,176,177,177,177,177,177,177,178,178,178,178,178,178,178,179,179,179,179,179,179,179,180,180,180,180,180,180,180,181,181,181,181,181,181,181,182,182,182,182,182,182,182,183,183,183,183,183,183,183,184,184,184,184,184,184,184,185,185,185,185,185,185,185,186,186,186,186,186,186,186,187,187,187,187,187,187,187,188,188,188,188,188,188,188,189,189,189,189,189,189,189,189,190,190,190,190,190,190,190,191,191,191,191,191,191,191,192,192,192,192,192,192,192,193,193,193,193,193,193,193,193,194,194,194,194,194,194,194,195,195,195,195,195,195,195,195,196,196,196,196,196,196,196,197,197,197,197,197,197,197,197,198,198,198,198,198,198,198,199,199,199,199,199,199,199,199,200,200,200,200,200,200,200,201,201,201,201,201,201,201,201,202,202,202,202,202,202,202,202,203,203,203,203,203,203,203,203,204,204,204,204,204,204,204,204,205,205,205,205,205,205,205,205,206,206,206,206,206,206,206,206,207,207,207,207,207,207,207,207,208,208,208,208,208,208,208,208,209,209,209,209,209,209,209,209,210,210,210,210,210,210,210,210,210,211,211,211,211,211,211,211,211,212,212,212,212,212,212,212,212,213,213,213,213,213,213,213,213,213,214,214,214,214,214,214,214,214,214,215,215,215,215,215,215,215,215,216,216,216,216,216,216,216,216,216,217,217,217,217,217,217,217,217,217,218,218,218,218,218,218,218,218,218,219,219,219,219,219,219,219,219,219,220,220,220,220,220,220,220,220,220,220,221,221,221,221,221,221,221,221,221,222,222,222,222,222,222,222,222,222,222,223,223,223,223,223,223,223,223,223,224,224,224,224,224,224,224,224,224,224,225,225,225,225,225,225,225,225,225,225,226,226,226,226,226,226,226,226,226,226,227,227,227,227,227,227,227,227,227,227,228,228,228,228,228,228,228,228,228,228,228,229,229,229,229,229,229,229,229,229,229,230,230,230,230,230,230,230,230,230,230,230,231,231,231,231,231,231,231,231,231,231,231,232,232,232,232,232,232,232,232,232,232,232,233,233,233,233,233,233,233,233,233,233,233,233,234,234,234,234,234,234,234,234,234,234,234,234,235,235,235,235,235,235,235,235,235,235,235,235,236,236,236,236,236,236,236,236,236,236,236,236,237,237,237,237,237,237,237,237,237,237,237,237,237,238,238,238,238,238,238,238,238,238,238,238,238,238,239,239,239,239,239,239,239,239,239,239,239,239,239,240,240,240,240,240,240,240,240,240,240,240,240,240,240,241,241,241,241,241,241,241,241,241,241,241,241,241,241,241,242,242,242,242,242,242,242,242,242,242,242,242,242,242,242,243,243,243,243,243,243,243,243,243,243,243,243,243,243,243,243,244,244,244,244,244,244,244,244,244,244,244,244,244,244,244,244,245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,246,246,246,246,246,246,246,246,246,246,246,246,246,246,246,246,246,246,247,247,247,247,247,247,247,247,247,247,247,247,247,247,247,247,247,247,247,247,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,254,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,252,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,251,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,249,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,248,247,247,247,247,247,247,247,247,247,247,247,247,247,247,247,247,247,247,247,247,246,246,246,246,246,246,246,246,246,246,246,246,246,246,246,246,246,246,245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,245,244,244,244,244,244,244,244,244,244,244,244,244,244,244,244,244,243,243,243,243,243,243,243,243,243,243,243,243,243,243,243,243,242,242,242,242,242,242,242,242,242,242,242,242,242,242,242,241,241,241,241,241,241,241,241,241,241,241,241,241,241,241,240,240,240,240,240,240,240,240,240,240,240,240,240,240,239,239,239,239,239,239,239,239,239,239,239,239,239,238,238,238,238,238,238,238,238,238,238,238,238,238,237,237,237,237,237,237,237,237,237,237,237,237,237,236,236,236,236,236,236,236,236,236,236,236,236,235,235,235,235,235,235,235,235,235,235,235,235,234,234,234,234,234,234,234,234,234,234,234,234,233,233,233,233,233,233,233,233,233,233,233,233,232,232,232,232,232,232,232,232,232,232,232,231,231,231,231,231,231,231,231,231,231,231,230,230,230,230,230,230,230,230,230,230,230,229,229,229,229,229,229,229,229,229,229,228,228,228,228,228,228,228,228,228,228,228,227,227,227,227,227,227,227,227,227,227,226,226,226,226,226,226,226,226,226,226,225,225,225,225,225,225,225,225,225,225,224,224,224,224,224,224,224,224,224,224,223,223,223,223,223,223,223,223,223,222,222,222,222,222,222,222,222,222,222,221,221,221,221,221,221,221,221,221,220,220,220,220,220,220,220,220,220,220,219,219,219,219,219,219,219,219,219,218,218,218,218,218,218,218,218,218,217,217,217,217,217,217,217,217,217,216,216,216,216,216,216,216,216,216,215,215,215,215,215,215,215,215,214,214,214,214,214,214,214,214,214,213,213,213,213,213,213,213,213,213,212,212,212,212,212,212,212,212,211,211,211,211,211,211,211,211,210,210,210,210,210,210,210,210,210,209,209,209,209,209,209,209,209,208,208,208,208,208,208,208,208,207,207,207,207,207,207,207,207,206,206,206,206,206,206,206,206,205,205,205,205,205,205,205,205,204,204,204,204,204,204,204,204,203,203,203,203,203,203,203,203,202,202,202,202,202,202,202,202,201,201,201,201,201,201,201,201,200,200,200,200,200,200,200,199,199,199,199,199,199,199,199,198,198,198,198,198,198,198,197,197,197,197,197,197,197,197,196,196,196,196,196,196,196,195,195,195,195,195,195,195,195,194,194,194,194,194,194,194,193,193,193,193,193,193,193,193,192,192,192,192,192,192,192,191,191,191,191,191,191,191,190,190,190,190,190,190,190,189,189,189,189,189,189,189,189,188,188,188,188,188,188,188,187,187,187,187,187,187,187,186,186,186,186,186,186,186,185,185,185,185,185,185,185,184,184,184,184,184,184,184,183,183,183,183,183,183,183,182,182,182,182,182,182,182,181,181,181,181,181,181,181,180,180,180,180,180,180,180,179,179,179,179,179,179,179,178,178,178,178,178,178,178,177,177,177,177,177,177,176,176,176,176,176,176,176,175,175,175,175,175,175,175,174,174,174,174,174,174,174,173,173,173,173,173,173,173,172,172,172,172,172,172,171,171,171,171,171,171,171,170,170,170,170,170,170,170,169,169,169,169,169,169,168,168,168,168,168,168,168,167,167,167,167,167,167,167,166,166,166,166,166,166,165,165,165,165,165,165,165,164,164,164,164,164,164,163,163,163,163,163,163,163,162,162,162,162,162,162,161,161,161,161,161,161,161,160,160,160,160,160,160,159,159,159,159,159,159,159,158,158,158,158,158,158,157,157,157,157,157,157,157,156,156,156,156,156,156,155,155,155,155,155,155,155,154,154,154,154,154,154,153,153,153,153,153,153,152,152,152,152,152,152,152,151,151,151,151,151,151,150,150,150,150,150,150,150,149,149,149,149,149,149,148,148,148,148,148,148,147,147,147,147,147,147,147,146,146,146,146,146,146,145,145,145,145,145,145,144,144,144,144,144,144,144,143,143,143,143,143,143,142,142,142,142,142,142,141,141,141,141,141,141,141,140,140,140,140,140,140,139,139,139,139,139,139,138,138,138,138,138,138,137,137,137,137,137,137,137,136,136,136,136,136,136,135,135,135,135,135,135,134,134,134,134,134,134,134,133,133,133,133,133,133,132,132,132,132,132,132,131,131,131,131,131,131,130,130,130,130,130,130,130,129,129,129,129,129,129,128,128,128,128,128,128,127,127,127,127,127,127,127,127,127,127,127,127,127,126,126,126,126,126,126,125,125,125,125,125,125,124,124,124,124,124,124,124,123,123,123,123,123,123,122,122,122,122,122,122,121,121,121,121,121,121,120,120,120,120,120,120,120,119,119,119,119,119,119,118,118,118,118,118,118,117,117,117,117,117,117,117,116,116,116,116,116,116,115,115,115,115,115,115,114,114,114,114,114,114,113,113,113,113,113,113,113,112,112,112,112,112,112,111,111,111,111,111,111,110,110,110,110,110,110,110,109,109,109,109,109,109,108,108,108,108,108,108,107,107,107,107,107,107,107,106,106,106,106,106,106,105,105,105,105,105,105,104,104,104,104,104,104,104,103,103,103,103,103,103,102,102,102,102,102,102,102,101,101,101,101,101,101,100,100,100,100,100,100,99,99,99,99,99,99,99,98,98,98,98,98,98,97,97,97,97,97,97,97,96,96,96,96,96,96,95,95,95,95,95,95,95,94,94,94,94,94,94,93,93,93,93,93,93,93,92,92,92,92,92,92,91,91,91,91,91,91,91,90,90,90,90,90,90,89,89,89,89,89,89,89,88,88,88,88,88,88,87,87,87,87,87,87,87,86,86,86,86,86,86,86,85,85,85,85,85,85,84,84,84,84,84,84,84,83,83,83,83,83,83,83,82,82,82,82,82,82,81,81,81,81,81,81,81,80,80,80,80,80,80,80,79,79,79,79,79,79,79,78,78,78,78,78,78,78,77,77,77,77,77,77,76,76,76,76,76,76,76,75,75,75,75,75,75,75,74,74,74,74,74,74,74,73,73,73,73,73,73,73,72,72,72,72,72,72,72,71,71,71,71,71,71,71,70,70,70,70,70,70,70,69,69,69,69,69,69,69,68,68,68,68,68,68,68,67,67,67,67,67,67,67,66,66,66,66,66,66,66,65,65,65,65,65,65,65,65,64,64,64,64,64,64,64,63,63,63,63,63,63,63,62,62,62,62,62,62,62,61,61,61,61,61,61,61,61,60,60,60,60,60,60,60,59,59,59,59,59,59,59,59,58,58,58,58,58,58,58,57,57,57,57,57,57,57,57,56,56,56,56,56,56,56,55,55,55,55,55,55,55,55,54,54,54,54,54,54,54,53,53,53,53,53,53,53,53,52,52,52,52,52,52,52,52,51,51,51,51,51,51,51,51,50,50,50,50,50,50,50,50,49,49,49,49,49,49,49,49,48,48,48,48,48,48,48,48,47,47,47,47,47,47,47,47,46,46,46,46,46,46,46,46,45,45,45,45,45,45,45,45,44,44,44,44,44,44,44,44,44,43,43,43,43,43,43,43,43,42,42,42,42,42,42,42,42,41,41,41,41,41,41,41,41,41,40,40,40,40,40,40,40,40,40,39,39,39,39,39,39,39,39,38,38,38,38,38,38,38,38,38,37,37,37,37,37,37,37,37,37,36,36,36,36,36,36,36,36,36,35,35,35,35,35,35,35,35,35,34,34,34,34,34,34,34,34,34,34,33,33,33,33,33,33,33,33,33,32,32,32,32,32,32,32,32,32,32,31,31,31,31,31,31,31,31,31,30,30,30,30,30,30,30,30,30,30,29,29,29,29,29,29,29,29,29,29,28,28,28,28,28,28,28,28,28,28,27,27,27,27,27,27,27,27,27,27,26,26,26,26,26,26,26,26,26,26,26,25,25,25,25,25,25,25,25,25,25,24,24,24,24,24,24,24,24,24,24,24,23,23,23,23,23,23,23,23,23,23,23,22,22,22,22,22,22,22,22,22,22,22,21,21,21,21,21,21,21,21,21,21,21,21,20,20,20,20,20,20,20,20,20,20,20,20,19,19,19,19,19,19,19,19,19,19,19,19,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,17,17,17,18,18,18,18,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,20,20,20,20,21,21,21,21,21,21,21,21,21,21,21,21,22,22,22,22,22,22,22,22,22,22,22,23,23,23,23,23,23,23,23,23,23,23,24,24,24,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,25,25,26,26,26,26,26,26,26,26,26,26,26,27,27,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,28,28,28,29,29,29,29,29,29,29,29,29,29,30,30,30,30,30,30,30,30,30,30,31,31,31,31,31,31,31,31,31,32,32,32,32,32,32,32,32,32,32,33,33,33,33,33,33,33,33,33,34,34,34,34,34,34,34,34,34,34,35,35,35,35,35,35,35,35,35,36,36,36,36,36,36,36,36,36,37,37,37,37,37,37,37,37,37,38,38,38,38,38,38,38,38,38,39,39,39,39,39,39,39,39,40,40,40,40,40,40,40,40,40,41,41,41,41,41,41,41,41,41,42,42,42,42,42,42,42,42,43,43,43,43,43,43,43,43,44,44,44,44,44,44,44,44,44,45,45,45,45,45,45,45,45,46,46,46,46,46,46,46,46,47,47,47,47,47,47,47,47,48,48,48,48,48,48,48,48,49,49,49,49,49,49,49,49,50,50,50,50,50,50,50,50,51,51,51,51,51,51,51,51,52,52,52,52,52,52,52,52,53,53,53,53,53,53,53,53,54,54,54,54,54,54,54,55,55,55,55,55,55,55,55,56,56,56,56,56,56,56,57,57,57,57,57,57,57,57,58,58,58,58,58,58,58,59,59,59,59,59,59,59,59,60,60,60,60,60,60,60,61,61,61,61,61,61,61,61,62,62,62,62,62,62,62,63,63,63,63,63,63,63,64,64,64,64,64,64,64,65,65,65,65,65,65,65,65,66,66,66,66,66,66,66,67,67,67,67,67,67,67,68,68,68,68,68,68,68,69,69,69,69,69,69,69,70,70,70,70,70,70,70,71,71,71,71,71,71,71,72,72,72,72,72,72,72,73,73,73,73,73,73,73,74,74,74,74,74,74,74,75,75,75,75,75,75,75,76,76,76,76,76,76,76,77,77,77,77,77,77,78,78,78,78,78,78,78,79,79,79,79,79,79,79,80,80,80,80,80,80,80,81,81,81,81,81,81,81,82,82,82,82,82,82,83,83,83,83,83,83,83,84,84,84,84,84,84,84,85,85,85,85,85,85,86,86,86,86,86,86,86,87,87,87,87,87,87,87,88,88,88,88,88,88,89,89,89,89,89,89,89,90,90,90,90,90,90,91,91,91,91,91,91,91,92,92,92,92,92,92,93,93,93,93,93,93,93,94,94,94,94,94,94,95,95,95,95,95,95,95,96,96,96,96,96,96,97,97,97,97,97,97,97,98,98,98,98,98,98,99,99,99,99,99,99,99,100,100,100,100,100,100,101,101,101,101,101,101,102,102,102,102,102,102,102,103,103,103,103,103,103,104,104,104,104,104,104,104,105,105,105,105,105,105,106,106,106,106,106,106,107,107,107,107,107,107,107,108,108,108,108,108,108,109,109,109,109,109,109,110,110,110,110,110,110,110,111,111,111,111,111,111,112,112,112,112,112,112,113,113,113,113,113,113,113,114,114,114,114,114,114,115,115,115,115,115,115,116,116,116,116,116,116,117,117,117,117,117,117,117,118,118,118,118,118,118,119,119,119,119,119,119,120,120,120,120,120,120,120,121,121,121,121,121,121,122,122,122,122,122,122,123,123,123,123,123,123,124,124,124,124,124,124,124,125,125,125,125,125,125,126,126,126,126,126,126,127,127,127,127,127,127,};
  int sineStep = 0;
  volatile int incrementAmount = 0; // how many indeces to step through the array every iteration
  
  // Other Wave Stuff
  volatile int waveStep = 0; // the current iteration step
  int switchIndex = 0; // after how many steps to switch wave direction
  boolean upPhase = true; // distinguish between wave phases

  // Angular Wave Stuff
  bool useIntEstimate = false;
  float waveIncreaseAmt = 0;
  float actualCurrentWaveAmp = 0;
  int increasePerStep = 0; // how much each step should increase (from 1 to 255)
  int stepsBeforeIncrease = 0; // after how many iterations an increase should occur
  int currentWaveAmp = 0;

  // ------ ENVELOPE VARIABLES ------
  int envCurrentStep = ENV_OFF;
  
  // in milliseconds
  int envCurrentTime = 0;
  unsigned long lastTime = 0;
  
  int envAttackTime = 0;
  int envDecayTime = 0;
  int envReleaseTime = 0;

  // in values from 0 to 255
  volatile byte envCurrentLevel = 0;
  const byte envMaxLevel = 80;
  byte envSustainLevel = 20;
  
  // ------ MIDI VARIABLES ------
  //Create an instance of the MIDI library
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

  int keyIndexToFreq[] = {16,17,18,19,21,22,23,25,26,28,29,31,33,35,37,39,41,44,46,49,52,55,58,62,65,69,73,78,82,87,93,98,104,110,117,123,131,139,147,156,165,175,185,196,208,220,233,247,262,277,294,311,330,349,370,392,415,440,466,494,523,554,587,622,659,698,740,784,831,880,932,988,1047,1109,1175,1245,1319,1397,1480,1568,1661,1760,1865,1976,2093,2217,2349,2489,2637,2794,2960,3136,3322,3520,3729,3951,4186,4435,4699,4978,5274,5588,5920,6272,6645,7040,7459,7902};
  int lastPitch = 0;
  
  // Initialize differnt modes
  void initModes() {
    modes[OFF_MODE].modeName = "Off";
    modes[OFF_MODE].numValues = 0;
    modes[OFF_MODE].values = malloc(sizeof(int));

    modes[PRESET_MODE].modeName = "Preset";
    modes[PRESET_MODE].numValues = NUM_PRESETS;
    modes[PRESET_MODE].values = malloc(NUM_PRESETS * sizeof(int));
    modes[PRESET_MODE].values[0] = 0;
    modes[PRESET_MODE].values[1] = 1;
    modes[PRESET_MODE].values[2] = 2;
    modes[PRESET_MODE].values[3] = 3;
    modes[PRESET_MODE].values[4] = 4;

    modes[WAVE_TYPE_MODE].modeName = "Wave Type";
    modes[WAVE_TYPE_MODE].numValues = 4;
    modes[WAVE_TYPE_MODE].values = malloc(4 * sizeof(int));
    modes[WAVE_TYPE_MODE].values[0] = SINE_SETTING;
    modes[WAVE_TYPE_MODE].values[1] = SQUARE_SETTING;
    modes[WAVE_TYPE_MODE].values[2] = TRI_SETTING;
    modes[WAVE_TYPE_MODE].values[3] = SAW_SETTING;

    waveTypeNames[SINE_SETTING] = "Sine";
    waveTypeNames[SQUARE_SETTING] = "Square";
    waveTypeNames[TRI_SETTING] = "Triangle";
    waveTypeNames[SAW_SETTING] = "Sawtooth";

    int envelope_millis[] = { 0, 100, 200, 300, 400, 500, 600, 700, 800, 900 };
    
    modes[ATTACK_MODE].modeName = "Attack";
    modes[ATTACK_MODE].numValues = 10;
    modes[ATTACK_MODE].values = malloc(10 * sizeof(int));
    for (int i = 0; i < 10; i++)
      modes[ATTACK_MODE].values[i] = envelope_millis[i];

    modes[DECAY_MODE].modeName = "Decay";
    modes[DECAY_MODE].numValues = 10;
    modes[DECAY_MODE].values = malloc(10 * sizeof(int));
    for (int i = 0; i < 10; i++)
      modes[DECAY_MODE].values[i] = envelope_millis[i];

    modes[RELEASE_MODE].modeName = "Release";
    modes[RELEASE_MODE].numValues = 10;
    modes[RELEASE_MODE].values = malloc(10 * sizeof(int));
    for (int i = 0; i < 10; i++)
      modes[RELEASE_MODE].values[i] = envelope_millis[i];

    int sustainLevels[] = { 2, 4, 6, 8, 10, 12, 14, 16, 18, 20 };

    modes[SUSTAIN_MODE].modeName = "Sustain";
    modes[SUSTAIN_MODE].numValues = 10;
    modes[SUSTAIN_MODE].values = malloc(10 * sizeof(int));
    for (int i = 0; i < 10; i++)
      modes[SUSTAIN_MODE].values[i] = sustainLevels[i];

    sustainLevelNames[0] = "10%";
    sustainLevelNames[1] = "20%";
    sustainLevelNames[2] = "30%";
    sustainLevelNames[3] = "40%";
    sustainLevelNames[4] = "50%";
    sustainLevelNames[5] = "60%";
    sustainLevelNames[6] = "70%";
    sustainLevelNames[7] = "80%";
    sustainLevelNames[8] = "90%";
    sustainLevelNames[9] = "100%";
  }
  
  void setup()
  {
    Serial.begin(9600);
    Serial.write("Starting\n");
    
    initModes();
    Serial.write("Modes initialized\n");

    // Set up LEDs
    pinMode(ledPin, OUTPUT);

    // Set up LCD
    if (usingLCD) {
      lcd.begin(16,2); // sixteen characters across - 2 lines
      lcd.backlight();
    }
    
    // Set interrupt as audio loop
    cli();//stop interrupts

    //set timer2 interrupt at 40kHz
    TCCR2A = 0;// set entire TCCR2A register to 0
    TCCR2B = 0;// same for TCCR2B
    TCNT2  = 0;//initialize counter value to 0
    // set compare match register for 40khz increments
    OCR2A = 49;// = (16*10^6) / (8*40000)-1
    // turn on CTC mode
    TCCR2A |= (1 << WGM21);
    // Set CS21 bit for 8 prescaler
    TCCR2B |= (1 << CS21); 
    // enable timer compare interrupt
    TIMSK2 |= (1 << OCIE2A);
   
    sei();//allow interrupts

    // Set PORTA bits
    if (usingDAC) {
      DDRA = B11111111;
    }

    MIDI.begin(MIDI_CHANNEL_OMNI); // Initialize the Midi Library.
    // OMNI sets it to listen to all channels.. MIDI.begin(2) would set it 
    // to respond to notes on channel 2 only.
    MIDI.setHandleNoteOn(MyHandleNoteOn); // This is important!! This command
    // tells the Midi Library which function you want to call when a NOTE ON command
    // is received. In this case it's "MyHandleNoteOn".
    MIDI.setHandleNoteOff(MyHandleNoteOff); // This command tells the Midi Library 
    // to call "MyHandleNoteOff" when a NOTE OFF command is received.

    Serial.write("Setup finished\n");

    updateLCD();
  }
  
  // Called by the Midi Library when a MIDI NOTE ON message is received.
  void MyHandleNoteOn(byte channel, byte pitch, byte velocity) {
    lastPitch = pitch;
    noteHit(keyIndexToFreq[pitch]);
    digitalWrite(ledPin, HIGH);  //Turn LED on
  }
  
  // Called by the Midi Library when a MIDI NOTE OFF message is received.
  // * A NOTE ON message with Velocity = 0 will be treated as a NOTE OFF message *
  void MyHandleNoteOff(byte channel, byte pitch, byte velocity) {
    if (pitch == lastPitch)
      goToEnvStep(ENV_RELEASE);
    digitalWrite(ledPin, LOW);  //Turn LED off
  }

  // Interrupt loop that generates audio
  ISR(TIMER2_COMPA_vect) {
    if (currentMode == OFF_MODE)
      PORTA = 0;
    else {
      switch (currentWaveType) {
        case SINE_SETTING:
          // Move sine index by appropriate amount
          sineStep += incrementAmount;

          // Reset at end of array
          if (sineStep >= precalculatedSteps) {
            sineStep = 0;
          }
          
          PORTA = min(sine[sineStep], envCurrentLevel);
          break;
          
        case SQUARE_SETTING:
          waveStep++;
          
          // Switch between up and down phases at appropriate intervals
          if (waveStep >= switchIndex) {
            waveStep = 0;
            upPhase = !upPhase;
          }
          
          PORTA = upPhase ? envCurrentLevel : 0;
          break;
          
        case SAW_SETTING:
          if (useIntEstimate) {
            waveStep++;
            // Increase amplitude at appropriate intervals
            if (waveStep >= stepsBeforeIncrease) {
              currentWaveAmp += increasePerStep;
              waveStep = 0;
            }
          } else {
            actualCurrentWaveAmp += waveIncreaseAmt;
            currentWaveAmp = (int)actualCurrentWaveAmp;
          }
          
          // Reset amplitude when it reaches its maximum
          if (currentWaveAmp > 255) {
            currentWaveAmp = 0;
            actualCurrentWaveAmp = 0;
          }
            
          PORTA = min(currentWaveAmp, envCurrentLevel);
          break;
          
        case TRI_SETTING:
          if (useIntEstimate) {
            waveStep++;
            // Increase or decrease amplitude at appropriate intervals
            if (waveStep >= stepsBeforeIncrease) {
              if (upPhase)
                currentWaveAmp += increasePerStep;
              else
                currentWaveAmp -= increasePerStep;
              waveStep = 0;
            }
          } else {
            if (upPhase)
              actualCurrentWaveAmp += waveIncreaseAmt;
            else
              actualCurrentWaveAmp -= waveIncreaseAmt;
            currentWaveAmp = (int)actualCurrentWaveAmp;
          }
          
          // Reverse direction at min and max amplitudes
          if (currentWaveAmp >= 255) {
            upPhase = false;
            currentWaveAmp = 255;
          } else if (currentWaveAmp <= 0) {
            upPhase = true;
            currentWaveAmp = 0;
          }
          
          PORTA = min(currentWaveAmp, envCurrentLevel);
          break;
      }
    }
  }

  // Called when a note is played
  void noteHit(int freq) {
    // Determine wave settings
    // audio sampling rate = 40k samples / sec
    float spc, inc;
    switch (currentWaveType) {
      case SINE_SETTING:
        // frequency = (sr / precalculatedSteps) * incrementAmount
        incrementAmount = int(round(freq / (40000.0 / precalculatedSteps)));
        sineStep = 0;
        break;
        
      case SQUARE_SETTING:
        // samples / cycle = sr / freq
        switchIndex = (40000.0 / freq) / 2;
        waveStep = 0;
        break;
        
      case SAW_SETTING:
      case TRI_SETTING:
        // samples / cycle = sr / freq
        spc = (40000.0 / freq);
        if (currentWaveType == TRI_SETTING)
          spc /= 2;
        // break up cycle into 255 intervals
        inc = 255 / spc;
        if (useIntEstimate) {
          // Determine increment and round off, accounting for >=1 and <1 slopes
          if (inc >= 1) {
            increasePerStep = int(round(inc));
            stepsBeforeIncrease = 1;
          } else {
            increasePerStep = 1;
            stepsBeforeIncrease = int(round(1 / inc));
          }
        } else {
          waveIncreaseAmt = inc;
        }
        break;
    }

    // Reset envelope
    envCurrentStep = ENV_ATTACK;
    envCurrentTime = 0;
    envCurrentLevel = 0;
  }

  void loadPreset(int p) {
    currentPreset = p;
    setWaveType(presets[p][0]);
    envAttackTime = presets[p][1];
    envDecayTime = presets[p][2];
    envSustainLevel = presets[p][3];
    envReleaseTime = presets[p][4];
  }

  // Set the current wave type
  void setWaveType(int newWaveType) {
    currentWaveType = newWaveType;
  }

  // Set an envelope setting
  void setEnvelope(int envMode, int newSetting) {
    int newVal = modes[envMode].values[newSetting];
    
    switch (envMode) {
      case ATTACK_MODE:
        envAttackTime = newVal;
        break;
      case DECAY_MODE:
        envDecayTime = newVal;
        break;
      case RELEASE_MODE:
        envReleaseTime = newVal;
        break;
      case SUSTAIN_MODE:
        envSustainLevel = newVal;
        break;
    }
  }

  void goToEnvStep(int newMode) {
    envCurrentStep = newMode;
    envCurrentTime = 0;
  }

  // Print the current mode and setting on the LCD
  void updateLCD() {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(modes[currentMode].modeName);
    lcd.setCursor(0,1);
    switch (currentMode) {
      case PRESET_MODE:
        lcd.print(currentPreset + 1); break;
      case WAVE_TYPE_MODE:
        lcd.print(waveTypeNames[currentWaveType]); break;
      case ATTACK_MODE:
        lcd.print(envAttackTime + String("ms")); break;
      case DECAY_MODE:
        lcd.print(envDecayTime + String("ms")); break;
      case SUSTAIN_MODE:
        lcd.print(envSustainLevel); break;
      case RELEASE_MODE:
        lcd.print(envReleaseTime + String("ms")); break;
    }
  }
  
  void loop()
  {    
    // Get MIDI input
    MIDI.read();
    
    // Get input from potentiometers
    if (usingPots) {
      int oldPotVal1 = potVal1;
      int oldPotVal2 = potVal2;
      potVal1 = floor(10 * (analogRead(pot1Pin) / maxPotVal));
      potVal2 = floor(10 * (analogRead(pot2Pin) / maxPotVal));

      int oldMode = currentMode;
      int oldSetting = currentSetting;
      
      if (potVal1 != oldPotVal1) {
        currentMode = floor(numModes * (analogRead(pot1Pin) / maxPotVal));
        currentMode = (numModes-1) - min(currentMode, numModes-1);
      } else if (potVal2 != oldPotVal2) {
        int numVals = modes[currentMode].numValues;
        currentSetting = floor(numVals * (analogRead(pot2Pin) / maxPotVal));
        currentSetting = (numVals-1) - min(currentSetting, numVals-1);

        if (currentMode == PRESET_MODE) {
          loadPreset(currentSetting);
        } else if (currentMode == WAVE_TYPE_MODE) {
          setWaveType(currentSetting);
        } else if (currentMode != OFF_MODE) {
          setEnvelope(currentMode, currentSetting);
        }
      }
      
      if (oldSetting != currentSetting || oldMode != currentMode) {
        updateLCD();
      }
    }

    // Update envelope
    if (currentMode != OFF_MODE) {
        unsigned long newTime = millis();
        envCurrentTime += int(newTime - lastTime);
        lastTime = newTime;
        
        switch (envCurrentStep) {
          case ENV_ATTACK:
            envCurrentLevel = int(((float)envCurrentTime / envAttackTime) * envMaxLevel);
            
            if (envCurrentTime >= envAttackTime) {
              goToEnvStep(ENV_DECAY);
            }
            break;
          case ENV_DECAY:
            envCurrentLevel = envMaxLevel - int(((float)envCurrentTime / envDecayTime) * (envMaxLevel - envSustainLevel));
            
            if (envCurrentTime >= envDecayTime) {
              goToEnvStep(ENV_SUSTAIN);
            }
            break;
          case ENV_RELEASE:
            envCurrentLevel = envSustainLevel - int(((float)envCurrentTime / envReleaseTime) * envSustainLevel);
            
            if (envCurrentTime >= envReleaseTime) {
              goToEnvStep(ENV_OFF);
            }
            break;
          case ENV_SUSTAIN:
            envCurrentLevel = envSustainLevel;
            break;
          case ENV_OFF:
            envCurrentLevel = 0;
            break;
        }
    }

    // Write to LEDs
    led1R = (led1R + 1) % 255;
    led1G = (led1G + 1) % 255;
    led1B = (led1B + 1) % 255;
    led2R = (led2R + 1) % 255;
    led2G = (led2G + 1) % 255;
    led2B = (led2B + 1) % 255;
    analogWrite(extLed1R, led1R);
    analogWrite(extLed1G, led1G);
    analogWrite(extLed1B, led1B);
    analogWrite(extLed2R, led2R);
    analogWrite(extLed2G, led2G);
    analogWrite(extLed2B, led2B);
    delay(10);
  }

