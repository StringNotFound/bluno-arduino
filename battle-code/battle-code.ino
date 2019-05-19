/*
   This example plays every .WAV file it finds on the SD card in a loop
*/
#include <WaveHC.h>
#include <WaveUtil.h>

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the volumes root directory
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

FatReader f;

uint8_t dirLevel; // indent level for file/dir names    (for prettyprinting)
dir_t dirBuf;     // buffer for directory reads

const int LIGHT_PIN = 7;
const int RECV_PIN = 8;
const int DELAY_TIME = 500;
// the last time that this system received a remote signal
unsigned int last_int_time;

/*
   Define macro to put error messages in flash memory
*/
#define error(msg) error_P(PSTR(msg))

// Function definitions (we define them here, but the code is below)
void play(FatReader &dir);

//////////////////////////////////// SETUP
void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps for debugging

  putstring_nl("\nWave test!");  // say we woke up!

  putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(FreeRam());

  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)
    error("Card init. failed!");  // Something went wrong, lets print out why
  }

  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);

  // Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {   // we have up to 5 slots to look in
    if (vol.init(card, part))
      break;                           // we found one, lets bail
  }
  if (part == 5) {                     // if we ended up not finding one  :(
    error("No valid FAT partition!");  // Something went wrong, lets print out why
  }

  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(), DEC);     // FAT16 or FAT32?

  // Try to open the root directory
  if (!root.openRoot(vol)) {
    error("Can't open root dir!");      // Something went wrong,
  }

  // Whew! We got past the tough parts.
  putstring_nl("Files found (* = fragmented):");

  // Print out all of the files in all the directories.
  root.ls(LS_R | LS_FLAG_FRAGMENTED);

  // setup the two pins we're using
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(RECV_PIN, INPUT);
}

//////////////////////////////////// LOOP
void loop() {
  // the light starts off
  digitalWrite(LIGHT_PIN, HIGH);

  Serial.println("Press remote to continue");
  wait_for_signal();

  Serial.println("Starting; lights on");
  digitalWrite(LIGHT_PIN, LOW);

  while (!is_interrupt()) {
    if (!wave.isplaying) {
      // if the audio has stopped playing, start it again
      playfile("CHANT.WAV");
    }
  }

  // curse has been cast
  //Serial.println("Curse");
  playcomplete("CURSE.WAV");

  delay(1000);

  //Serial.println("Playing combat music");
  // the battle starts
  //playfile("CULTBAT.WAV");
  while (!is_interrupt()) {
    if (!wave.isplaying) {
      // if the wave has stopped playing, start it again
      playfile("CULTBAT.WAV");
    }
  }

  //Serial.println("Death Sequence");
  playfile("BLUNODIE.WAV");
  while (wave.isplaying) {
    digitalWrite(LIGHT_PIN, LOW);
    delay(100);
    digitalWrite(LIGHT_PIN, HIGH);
    delay(100);
  }

  delay(5000);
  playcomplete("VICTORY.WAV");

  // hang at the end
  while (true) {
    ;
  }
}

void wait_for_signal() {
  while (!is_interrupt()) {
    ;
  }
}

// Returns 1 if there is currently an interrupt, 0 else
int is_interrupt() {
  int interrupt = !digitalRead(RECV_PIN);
  unsigned int cur_time = millis();
  if ((cur_time - last_int_time > DELAY_TIME) && interrupt) {
    last_int_time = cur_time;
    return 1;
  }
  return 0;
}

// -------------------------AUDIO STUFF------------------------------

// Plays a full file from beginning to end with no pause.
void playcomplete(char *name) {
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
    // do nothing while its playing
  }
  // now its done playing
}

void playfile(char *name) {
  //FatReader f;
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  int success = f.open(root, name);
  if (!success) {
    Serial.println(success);
    putstring("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV"); return;
  }

  // ok time to play! start playback
  wave.play();
}

/////////////////////////////////// HELPERS
/*
   print error message and halt
*/
void error_P(const char *str) {
  PgmPrint("Error: ");
  SerialPrint_P(str);
  sdErrorCheck();
  while (1);
}
/*
   print error message and halt if SD I/O error, great for debugging!
*/
void sdErrorCheck(void) {
  if (!card.errorCode()) return;
  PgmPrint("\r\nSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  PgmPrint(", ");
  Serial.println(card.errorData(), HEX);
  while (1);
}
