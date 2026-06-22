#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>
#include <SPI.h>

#define TFT_CS 1
#define TFT_RST 2
#define TFT_DC 3
#define TFT_SCLK 4
#define TFT_MOSI 5

#define ROTATRY_CLK 10
#define ROTATRY_DT 20

int prev_btn = 6;
int play_btn = 7;
int next_btn = 8;

bool lastPrevState = HIGH;
bool lastPlayState = HIGH;
bool lastNextState = HIGH;

int lastClkState;
int currentVolume = 50;

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 2000;

unsigned long lastTickTime = 0;
long currentProgressMs = 0;
long totalDurationMs = 240000;

String lastArtist;
String lastTrackname;

char* SSID = "Bablu";
char* PASSWORD = "Ravisumit";
const char* CLIENT_ID = "0e2d5fca3d0b4d6d8c92e9e011d6374d";
const char* CLIENT_SECRET = "1d1f870dace54b358bcce9a7c5082176";

Spotify sp(CLIENT_ID, CLIENT_SECRET);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void setup() {
  Serial.begin(115200);

  pinMode(prev_btn, INPUT_PULLUP);
  pinMode(play_btn, INPUT_PULLUP);
  pinMode(next_btn, INPUT_PULLUP);

  pinMode(ROTATRY_CLK, INPUT_PULLUP);
  pinMode(ROTATRY_DT, INPUT_PULLUP);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  Serial.println("TFT Initialized!");
  tft.fillScreen(ST77XX_BLACK);


  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED)
  {
      delay(1000);
      Serial.print(".");
  }
  Serial.printf("\nConnected!\n");

  sp.begin();
  while(!sp.is_auth())
  {
      sp.handle_client();
  }
  Serial.println("Connected to Spotify!");

tft.setCursor(0,0);
tft.write(WiFi.localIP().toString().c_str());
}

void loop() {

  bool currentPrevState = digitalRead(prev_btn);
  bool currentPlayState = digitalRead(play_btn);
  bool currentNextState = digitalRead(next_btn);


  if (currentPrevState == LOW && lastPrevState == HIGH){
    Serial.println("Button Press: Previous");
    sp.skip_to_previous();
    delay(200);
  }
  lastPrevState = currentPrevState;

  if (currentPlayState == LOW && lastPlayState == HIGH){
    Serial.println("Button Pressed: Play/Pause");

    if (sp.is_playing()){
      sp.pause_playback();
    } 
    // else{
    //   // sp.res
    // }
    delay(200);
  }
  lastPlayState = currentPlayState;

  if(currentNextState == LOW && lastNextState == HIGH){
    Serial.println("Button Pressed: Next");
    sp.skip_to_next();
    delay(200);
  }
  lastNextState = currentNextState;

  int clkState = digitalRead(ROTATRY_CLK);
  if (clkState != lastClkState && clkState == LOW){
    if (digitalRead(ROTATRY_DT) != clkState){
      currentVolume = min(100, currentVolume + 5);
    } else{
      currentVolume = max(0, currentVolume - 5);
    }
    sp.set_volume(currentVolume);
    delay(50);
  }
  lastClkState = clkState;

  if(millis() - lastTickTime >= 1000){
    lastTickTime = millis();
    if(currentProgressMs < totalDurationMs){
      currentProgressMs += 1000;

      int maxBarWidth = 140;
      int barX = 10;
      int barY = 80;
      int progressWidth = (currentProgressMs * maxBarWidth) / totalDurationMs;

      tft.fillRect(barX, barY, progressWidth, 4, ST77XX_GREEN);
      tft.fillRect(barX + progressWidth, barY, maxBarWidth - progressWidth, 4, 0x4208);
    }
  }


  if(millis() - lastUpdateTime >= updateInterval){

  lastUpdateTime = millis();
  String currentArtist = sp.current_artist_names();
  String currentTrackname = sp.current_track_name();

  if (lastArtist != currentArtist && currentArtist != "Something went wrong" && !currentArtist.isEmpty()) {
        tft.fillScreen(ST77XX_BLACK);
        lastArtist = currentArtist;
        Serial.println("Artist: " + lastArtist);
        tft.setCursor(10,10);
        tft.write(lastArtist.c_str());
    }

  if (lastTrackname != currentTrackname && currentTrackname != "Something went wrong" && currentTrackname != "null") {
        lastTrackname = currentTrackname;
        Serial.println("Track: " + lastTrackname);
        tft.setCursor(10,40);
        tft.write(lastTrackname.c_str());
    }
  }
}
