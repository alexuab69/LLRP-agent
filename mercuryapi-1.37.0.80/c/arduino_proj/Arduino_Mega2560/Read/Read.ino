#include <tm_reader.h>

#define ERROR_BLINK_COUNT 10
#define ERROR_BLINK_INTERVAL 100
#define TAG_SEARCH_TIME 500 //ms
#define CONSOLE_BAUD_RATE 115200

const int sysLedPin = 13;
static int ledState = LOW;  // ledState used to set the LED
static HardwareSerial *console = &Serial;

TMR_Reader r, *rp;
TMR_ReadListenerBlock rlb;
TMR_ReadExceptionListenerBlock reb;

char string[100];
TMR_String model;
uint32_t count = 0;
bool stopReadCommandSent = false;

TMR_Status parseSingleThreadedResponse(TMR_Reader* rp, uint32_t readTime);
void notify_read_listeners(TMR_Reader *reader, TMR_TagReadData *trd);
void notify_exception_listeners(TMR_Reader *reader, TMR_Status status);
void reset_continuous_reading(struct TMR_Reader *reader, bool dueToError);
void readCallback(TMR_Reader *reader, const TMR_TagReadData *t, void *cookie);
void exceptionCallback(TMR_Reader *reader, TMR_Status error, void *cookie);

static void blink(int count, int blinkInterval)
{
  unsigned long blinkTime;
  unsigned long currentTime;

  blinkTime = 0;
  currentTime = millis();

  while (count)
  {
    if (currentTime > blinkTime) {
      // save the last time you blinked the LED
      blinkTime = currentTime + blinkInterval;

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW)
      {ledState = HIGH;}
      else
      {
        ledState = LOW;
        --count;
      }

      // set the LED with the ledState of the variable:
      digitalWrite(sysLedPin, ledState);
    }

    currentTime = millis();
  }
}

static void checkerr(TMR_Reader *rp, TMR_Status ret, int exitval, const char *msg)
{
  if (TMR_SUCCESS != ret)
  {
    console->print("ERROR ");
    console->print(msg);
    console->print(": 0x");
    console->print(ret, HEX);
    console->print(": ");
    // console->println(TMR_strerror(ret));
    while (1)
    {
      blink(ERROR_BLINK_COUNT, ERROR_BLINK_INTERVAL);
    }
  }
}

static void printComment(char *msg)
{
  console->print("#");
  console->println(msg);
}

void readCallback(TMR_Reader *reader, const TMR_TagReadData *trd, void *cookie) {
  char epcStr[128];
  char timeStr[128];

  TMR_bytesToHex(trd->tag.epc, trd->tag.epcByteCount, epcStr);
  console->print("TAGREAD EPC:");
  console->print(epcStr);

  reportMetaData(*trd);
  console->print("\n");
}

void exceptionCallback(TMR_Reader *reader, TMR_Status error, void *cookie) 
{
  console->println("Async read failed.");
}

static void reportMetaData(TMR_TagReadData trd)
{
  if (TMR_TRD_METADATA_FLAG_PROTOCOL & trd.metadataFlags)
  {
    console->print(" PROTOCOL:");
    console->print(trd.tag.protocol);
  }

  if (TMR_TRD_METADATA_FLAG_ANTENNAID & trd.metadataFlags)
  {
    console->print(" ANT:");
    console->print(trd.antenna);
  }

  if (TMR_TRD_METADATA_FLAG_READCOUNT & trd.metadataFlags)
  {
    console->print(" READCOUNT:");
    console->print(trd.readCount);
  }
}

TMR_Status
parseSingleThreadedResponse(TMR_Reader* rp, uint32_t readTime)
{
  TMR_Status ret = TMR_SUCCESS;
  uint32_t elapsedTime = 0;
  uint64_t startTime = tmr_gettime();

  while (true)
  {
    TMR_TagReadData trd;

    ret = TMR_hasMoreTags(rp);
    if (TMR_SUCCESS == ret)
    {
      TMR_getNextTag(rp, &trd);
      notify_read_listeners(rp, &trd);
    }
    else if (TMR_ERROR_END_OF_READING == ret)
    {break;}
    else
    {
      if ((TMR_ERROR_NO_TAGS != ret) && (TMR_ERROR_NO_TAGS_FOUND != ret))
      {
        notify_exception_listeners(rp, ret);
      }
    }

    elapsedTime = tmr_gettime() - startTime;

    if ((elapsedTime > readTime) && (!stopReadCommandSent))
    {
      ret = TMR_stopReading(rp);
      if (TMR_SUCCESS == ret)
      {
        stopReadCommandSent = true;
      }
    }
  }
  reset_continuous_reading(rp);
  
  return TMR_SUCCESS;
}

static void continuousRead()
{
  TMR_Status ret;

  rlb.listener = readCallback;
  rlb.cookie = NULL;

  reb.listener = exceptionCallback;
  reb.cookie = NULL;

  ret = TMR_addReadListener(rp, &rlb);
  checkerr(rp, ret, 1, "adding read listener");

  ret = TMR_addReadExceptionListener(rp, &reb);
  checkerr(rp, ret, 1, "adding exception listener");

  ret = TMR_startReading(rp);
  checkerr(rp, ret, 1, "Start reading");
  
  parseSingleThreadedResponse(rp, TAG_SEARCH_TIME);
}

static void timedRead()
{
  TMR_TagReadData trd;
  char epcStr[128];
  TMR_Status ret;

  ret = TMR_read(rp, TAG_SEARCH_TIME, NULL);
  checkerr(rp, ret, 1, "reading tags");

  while (TMR_SUCCESS == TMR_hasMoreTags(rp))
  {
    ret = TMR_getNextTag(rp, &trd);
    checkerr(rp, ret, 1, "fetching tag");

    TMR_bytesToHex(trd.tag.epc, trd.tag.epcByteCount, epcStr);
    console->print("TAGREAD EPC:");
    console->print(epcStr);

    reportMetaData(trd);
    console->print("\n");
  }
}

static void readTags()
{
  TMR_Status ret;
  TMR_ReadPlan plan;
  uint8_t antennaList[] = { 1 };
  uint8_t antennaCount = 1;
  TMR_TagProtocol protocol;

  if (0 != strcmp("M3e", model.value))
  {protocol = TMR_TAG_PROTOCOL_GEN2;}
  else
  {protocol =TMR_TAG_PROTOCOL_ISO14443A;}

  //initializing the Read Plan
  TMR_RP_init_simple(&plan, antennaCount, antennaList, protocol, 100);

  //Commit the Read Plan
  ret = TMR_paramSet(rp, TMR_PARAM_READ_PLAN, &plan);
  checkerr(rp, ret, 1, "setting read plan");

  console->println("!!! Perform timed read !!!");
  timedRead();

  console->println("\n!!! Perform continuous read !!!");
  continuousRead();
}

static void initializeReader()
{
  TMR_Status ret;

  rp = &r;
  ret = TMR_create(rp, "tmr:///Serial1");
  checkerr(rp, ret, 1, "creating reader");

  ret = TMR_connect(rp);
  checkerr(rp, ret, 1, "connecting reader");

  model.value = string;
  model.max   = sizeof(string);
  TMR_paramGet(rp, TMR_PARAM_VERSION_MODEL, &model);
  checkerr(rp, ret, 1, "Getting version model");

  if (0 != strcmp("M3e", model.value))
  // Set region to North America
  {
    TMR_Region region = TMR_REGION_NA;
    ret = TMR_paramSet(rp, TMR_PARAM_REGION_ID, &region);
    checkerr(rp, ret, 1, "setting region");
  }
}

void setup()
{
  // Set the GPIO pins that we are concerned about to output
  pinMode(sysLedPin, OUTPUT);

  // initialize the GPIO pins
  digitalWrite(sysLedPin, 0);

  // start the console interface
  console->begin(CONSOLE_BAUD_RATE);

  //Initialize the RFID Module
  initializeReader();

  //Perform read and report tags
  readTags();

  TMR_destroy(rp);
}

void loop() {
}