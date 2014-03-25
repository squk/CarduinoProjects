typedef struct
{
  int codeType; // The type of code
  unsigned long codeValue; // The code value if not raw
  unsigned int rawCodes[RAWBUF]; // The durations if raw
  int codeLen; // The length of the code
  int toggle; // The RC5/6 toggle state
} 
IRCode;

/* Xbox Remote IRCodes*/
/*
IRCode whiteCode = { 1, 0xFFA857, {0}, 32, 0};
IRCode redCode = { 1, 0xFF9867, {0}, 32, 0};
IRCode greenCode = { 1, 0xFFD827, {0}, 32, 0};
IRCode blueCode = { 1, 0xFF8877, {0}, 32, 0};

IRCode purpleCode = { 1, 0xFFF00F, {0}, 32, 0};
IRCode amberCode = { 1, 0xFFE817, {0}, 32, 0};
IRCode cyanCode = { 1, 0xFF28D7, {0}, 32, 0};
IRCode yellowCode = { 1, 0xFF38C7, {0}, 32, 0};

IRCode strobeCode = { 1, 0xFF00FF, {0}, 32, 0};
IRCode cycleCode = { 1, 0xFFB24D, {0}, 32, 0};
IRCode fadeCode = { 1, 0xFF58A7, {0}, 32, 0};

IRCode onCode = { 1, 0xFFB04F, {0}, 32, 0};
IRCode offCode = { 1, 0xFFF807, {0}, 32, 0};
*/
unsigned long whiteCode = 0xFFA857;
unsigned long redCode = 0xFF9867;
unsigned long greenCode = 0xFFD827;
unsigned long blueCode = 0xFF8877;

unsigned long purpleCode = 0xFFF00F;
unsigned long amberCode = 0xFFE817;
unsigned long cyanCode = 0xFF28D7;
unsigned long yellowCode = 0xFF38C7;

unsigned long strobeCode = 0xFF00FF;
unsigned long cycleCode = 0xFFB24D;
unsigned long fadeCode = 0xFF58A7;

unsigned long onCode = 0xFFB04F;
unsigned long offCode = 0xFFF807;
