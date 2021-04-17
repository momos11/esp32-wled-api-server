#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <string>

#define LEDPIN 12
#define NUMPIXELS 60

const char *ssid = "57Dps3P";
const char *password = "3p7SD#m$87sa5k?=7HG";
WebServer server(80);
String lastState = "";
int ledState = HIGH;
String ledMode = "";
String colorString = "";
String side = "";
String colorLeft = "";
String colorRight = "";
String toggleState = "";
String brightness = "";
int rgb[3] = {};
unsigned long previousMillis = 0; // will store last time LED was updated
long OnTime = 250;                // milliseconds of on-time
long OffTime = 750;               // milliseconds of off-time

// Pattern types supported:
enum pattern {
    NONE,
    RAINBOW_CYCLE,
    THEATER_CHASE,
    COLOR_WIPE,
    SCANNER,
    FADE,
    STATIC,
    FADE_HORIZONTAL,
    TWINKLE,
    METEOR
};
// Patern directions supported:
enum direction {
    FORWARD,
    REVERSE
};

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel {
public:
    // Member Variables:
    pattern ActivePattern; // which pattern is running
    direction Direction;   // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    uint32_t Color1, Color2; // What colors are in use
    uint16_t TotalSteps;     // total number of steps in the pattern
    uint16_t Index;          // current step within the pattern

    void (*OnComplete)(); // Callback on completion of pattern
    int Wheel(byte WheelPos);

    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
            : Adafruit_NeoPixel(pixels, pin, type) {
        OnComplete = callback;
    }

    // Update the pattern
    void Update() {
        if ((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch (ActivePattern) {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                case STATIC:
                    StaticUpdate();
                    break;
                case METEOR:
                    MeteorUpdate();
                    break;
                case FADE_HORIZONTAL:
                    FadeHorizontalUpdate();
                    break;
                case TWINKLE:
                    TwinkleUpdate();
                    break;
                default:
                    break;
            }
        }
    }

    // Increment the Index and reset at the end
    void Increment() {
        if (Direction == FORWARD) {
            Index++;
            if (Index >= TotalSteps) {
                Index = 0;
                if (OnComplete != NULL) {
                    OnComplete(); // call the comlpetion callback
                }
            }
        } else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0) {
                Index = TotalSteps - 1;
                if (OnComplete != NULL) {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }

    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD) {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate() {
        for (int i = 0; i < numPixels(); i++) {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD) {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate() {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD) {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = NUMPIXELS;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }

    // Update the Theater Chase Pattern
    void TheaterChaseUpdate() {
        for (int i = 0; i < numPixels(); i++) {
            if ((i + Index) % 3 == 0) {
                setPixelColor(i, Color1);
            } else {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }

    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval) {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
    }

    // Update the Scanner Pattern
    void ScannerUpdate() {
        for (int i = 0; i < numPixels(); i++) {
            if (i == Index) // first half of the scan
            {
                Serial.print(i);
                setPixelColor(i, Color1);
            } else if (i == TotalSteps - Index) // The return trip.
            {
                Serial.print(i);
                setPixelColor(i, Color1);
            } else // fade to black
            {
                setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }

    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD) {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }

    // Update the Fade Pattern
    void FadeUpdate() {
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }

    void Static(uint8_t interval, direction dir = FORWARD) {
        ActivePattern = STATIC;
        Interval = 500;
        TotalSteps = 1;
        Index = 0;
        Direction = dir;
    }

    void StaticUpdate() {
        ColorSet(Color1);
        show();
    }

    void FadeHorizontal(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD) {
        ActivePattern = FADE_HORIZONTAL;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }

    void FadeHorizontalUpdate() {
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

        for (int i = 0; i < NUMPIXELS; i++) {
            setPixelColor(i, Color(red,green,blue));
            show();
        }

        //ColorSet(Color(red, green, blue));
        show();
        Increment();
    }

    void Twinkle(uint32_t color1, uint8_t interval) {
        ActivePattern = TWINKLE;
        Interval = interval;
        Color1 = color1;
        Index = 0;
    }

    void TwinkleUpdate() {
        ColorSet(Color2);

        for (int i = 0; i < 10; i++) {
            setPixelColor(random(NUMPIXELS), Color1);
            show();
        }

        show();
        Increment();
    }


    void Meteor(uint32_t color1, uint8_t interval) {
        ActivePattern = METEOR;
        Interval = interval;
        Color1 = color1;
        Index = 0;
    }

    void MeteorUpdate() {
        byte meteorSize = 20;
        byte meteorTrailDecay = 10;
        boolean meteorRandomDecay = false;
        ColorSet(0);

        for (int i = 0; i < NUMPIXELS + NUMPIXELS; i++) {

            for (int j = 0; j < NUMPIXELS; j++) {
                if ((!meteorRandomDecay) || (random(10) > 5)) {
                    fadeToBlack(j, meteorTrailDecay);
                }
            }

            // draw meteor
            for (int j = 0; j < meteorSize; j++) {
                if ((i - j < NUMPIXELS) && (i - j >= 0)) {
                    setPixelColor(i - j, Color1);
                }
            }
            show();
        }

        show();
        Increment();
    }

    void fadeToBlack(int ledNo, byte fadeValue) {
        uint32_t oldColor;
        uint8_t r, g, b;

        oldColor = getPixelColor(ledNo);
        r = (oldColor & 0x00ff0000UL) >> 16;
        g = (oldColor & 0x0000ff00UL) >> 8;
        b = (oldColor & 0x000000ffUL);

        r = (r <= 10) ? 0 : (int) r - (r * fadeValue / 256);
        g = (g <= 10) ? 0 : (int) g - (g * fadeValue / 256);
        b = (b <= 10) ? 0 : (int) b - (b * fadeValue / 256);

        setPixelColor(ledNo, r, g, b);

        setPixelColor(ledNo, DimColor(oldColor));
        //leds[ledNo].fadeToBlackBy( fadeValue );

    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color) {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color) {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color) {
        return color & 0xFF;
    }

    // Return color, dimmed by 75% (used by scanner)
    uint32_t DimColor(uint32_t color) {
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }

    // Reverse direction of the pattern
    void Reverse() {
        if (Direction == FORWARD) {
            Direction = REVERSE;
            Index = TotalSteps - 1;
        } else {
            Direction = FORWARD;
            Index = 0;
        }
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color) {
        for (int i = 0; i < numPixels(); i++) {
            setPixelColor(i, color);
        }
        show();
    }
};

int NeoPatterns::Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}


NeoPatterns Strip(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800, &StripComplete);

void StripComplete() {
    if (Strip.ActivePattern == FADE) {
        Strip.Reverse();
    }
}

void changeActivepattern(pattern newPattern) {
    Strip.ActivePattern = newPattern;
}

void changeActivepatternHelper() {
}

void changeActivepatternHelper1() {
}


void setCrossOrigin() {
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
};

int *colorConverter(String hexString) {
    unsigned int hexValue = (int) strtol(&hexString[1], NULL, 16);
    Serial.println(hexValue);
    int red = hexValue >> 16;
    int green = hexValue >> 8 & 0xFF;
    int blue = hexValue & 0xFF;

    rgb[0] = red;
    rgb[1] = green;
    rgb[2] = blue;
    Serial.println(red);
    Serial.println(green);
    Serial.println(blue);
    return rgb;
}

void handlePattern() {
    if (ledMode == "ON") {
        Strip.setBrightness(255);
        Strip.show();
    }
    if (ledMode == "OFF") {
        Strip.setBrightness(0);
        Strip.show();
    }
    if (ledMode == "RAINBOW_CYCLE") {
        Strip.Interval = 10;
        Strip.TotalSteps = 255;
        changeActivepattern(RAINBOW_CYCLE);
    }
    if (ledMode == "FADE") {
        Strip.Interval = 10;
        Strip.TotalSteps = 255;
        changeActivepattern(FADE);
    }

    if (ledMode == "BRIGHTNESS") {
        Strip.setBrightness(brightness.toInt());
    }

    if (ledMode == "STATIC") {
        changeActivepattern(STATIC);
    }

    if (ledMode == "SCANNER") {
        changeActivepattern(SCANNER);
    }

    if (ledMode == "THEATERCHASE") {
        changeActivepattern(THEATER_CHASE);
    }

    if (ledMode == "COLORWIPE") {
        changeActivepattern(COLOR_WIPE);
    }

    if (ledMode == "METEOR") {
        changeActivepattern(METEOR);
    }

    if (ledMode == "TWINKLE") {
        changeActivepattern(TWINKLE);
    }

    if (ledMode == "FADE_HORIZONTAL") {
        Strip.Interval = 10;
        Strip.TotalSteps = 255;
        changeActivepattern(FADE_HORIZONTAL);
    }

    if (ledMode == "COLOR") {
        Serial.println(colorString);
        Serial.println(side);

        colorConverter(colorString);

        if (side == "color1") {
            Strip.Color1 = Strip.Color(rgb[0], rgb[1], rgb[2]);
        }

        if (side == "color2") {
            Strip.Color2 = Strip.Color(rgb[0], rgb[1], rgb[2]);
        }
    }
}


void handleSubmit() { //Handler for the body path

    if (!server.hasArg("plain")) { //Check if body received

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    ledMode = doc["ledMode"].as<String>();
    colorString = doc["color"].as<String>();
    side = doc["side"].as<String>();
    brightness = doc["brightness"].as<String>();
    String message = ledMode;
    Serial.println(ledMode);
    handlePattern();

    server.send(200, "text/plain", message);
}

void handleGet() {

}

void wifiConnect() {
    WiFi.begin(ssid, password); // Connect to the network
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println(" ...");
    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
        delay(2000);
        Serial.print(WiFi.status());
        Serial.print('\n');
    }
    Serial.println('\n');
    Serial.println("Connection established!");
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);

    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);

    delay(10);
    Serial.println('\n');

    wifiConnect();

    server.on("/submit", HTTP_POST, handleSubmit);
    server.on("/get", HTTP_POST, handleGet);

    server.begin();
    Strip.begin();
}

void loop() {
    server.handleClient();
    Strip.Update();
}
