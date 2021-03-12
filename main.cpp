#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#define LEDPIN 0

const char *ssid = "57Dps3P";
const char *password = "3p7SD#m$87sa5k?=7HG";
WebServer server(80);
String led0 = "<a href=\"/led_an\">LED An</a>";
String led1 = "<a href=\"/led_aus\">LED Aus</a>";
String lastState = "";
int ledState = HIGH;
String ledMode = "";
String toggleState = "";
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
    BLINK,
    ON,
    OFF
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
                case BLINK:
                    BlinkUpdate(lastUpdate);
                    break;
                case ON:
                    OnUpdate();
                    break;
                case OFF:
                    OffUpdate();
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

    void Blink(uint8_t interval, direction dir = FORWARD) {
        ActivePattern = BLINK;
        Interval = interval;
        TotalSteps = 1;
        Index = 0;
        Direction = dir;
    }

    void BlinkUpdate(unsigned long currentMillis) {
        // digitalWrite(LEDPIN, HIGH); // Update the actual LED
        if ((ledState == HIGH) && (currentMillis - previousMillis >= OnTime)) {
            ledState = LOW;                 // Turn it off
            previousMillis = currentMillis; // Remember the time
            digitalWrite(LEDPIN, ledState); // Update the actual LED
        } else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime)) {
            ledState = HIGH;                // turn it on
            previousMillis = currentMillis; // Remember the time
            digitalWrite(LEDPIN, ledState); // Update the actual LED
        }
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
        TotalSteps = numPixels();
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

    void On(uint8_t interval, direction dir = FORWARD) {
        ActivePattern = ON;
        Interval = interval;
        TotalSteps = 1;
        Index = 0;
        Direction = dir;
    }

    void OnUpdate() {
        digitalWrite(LEDPIN, HIGH); // Update the actual LED
    }

    void Off(uint8_t interval, direction dir = FORWARD) {
        ActivePattern = OFF;
        Interval = interval;
        TotalSteps = 1;
        Index = 0;
        Direction = dir;
    }

    void OffUpdate() {
        digitalWrite(LEDPIN, LOW); // Update the actual LED
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

void StripComplete() {
}

NeoPatterns Strip(16, 0, NEO_GRB + NEO_KHZ800, &StripComplete);

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

void handlePattern(const String &mode) {
    if (mode == "ON") {
        Strip.ActivePattern = ON;
    }
    if (mode == "OFF") {
        Strip.ActivePattern = OFF;
    }
    if (mode == "BLINK") {
        Strip.ActivePattern = BLINK;
    }
    if (mode == "RAINBOW_CYCLE") {
        Strip.ActivePattern = RAINBOW_CYCLE;
    }
}

void handleBody() { //Handler for the body path

    if (server.hasArg("plain") == false) { //Check if body received

        server.send(200, "text/plain", "Body not received");
        return;
    }
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    ledMode = doc["ledMode"].as<String>();
    String message = ledMode;

    handlePattern(ledMode);

    server.send(200, "text/plain", message);
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

    //SERVER CONFIG
    // server.on("/", HTTP_GET, []() {
    //     server.send(200, F("text/html"),
    //                 F("Welcome to the REST Web Server"));
    // });
    server.on("/body", HTTP_POST, handleBody);
    server.on("/led_an", HTTP_POST, changeActivepatternHelper);
    server.on("/led_aus", HTTP_POST, changeActivepatternHelper1);
    server.begin();
    Strip.begin();
    Strip.Blink(500);
}

void loop() {
    server.handleClient();

    // if (toggleState == "off")
    // {
    //     digitalWrite(LEDPIN, LOW);
    // }

    Strip.Update();

    // if (toggleState == "on")
    //   if (ledMode == "BLINK")
    //     if((ledState == HIGH) && (currentMillis - previousMillis >= OnTime))
    //     {
    //       ledState = LOW;  // Turn it off
    //       previousMillis = currentMillis;  // Remember the time
    //       digitalWrite(LEDPIN, ledState);  // Update the actual LED
    //     }
    //     else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime))
    //     {
    //       ledState = HIGH;  // turn it on
    //       previousMillis = currentMillis;   // Remember the time
    //       digitalWrite(LEDPIN, ledState);	  // Update the actual LED
    //     }
    //   if(ledMode == "STATIC"){
    //     digitalWrite(LEDPIN, HIGH);
    //   }
}
