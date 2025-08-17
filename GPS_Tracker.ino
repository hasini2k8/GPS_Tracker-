#include <SoftwareSerial.h>

static const int GPS_RX_PIN = 4;
static const int GPS_TX_PIN = 3;
static const uint32_t GPS_BAUD_RATE = 9600;

SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);

String nmeaSentence = "";

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(GPS_BAUD_RATE);
  Serial.println("Time,Latitude,Longitude"); // CSV header
}

void loop() {
  while (gpsSerial.available()) {
    char c = gpsSerial.read();

    if (c == '\n') {
      processNMEASentence(nmeaSentence);
      nmeaSentence = "";
    } else if (c != '\r') {
      nmeaSentence += c;
    }
  }
}

void processNMEASentence(String sentence) {
  if (sentence.startsWith("$GPRMC")) {
    // Split by comma
    int fieldIndex = 0;
    int lastComma = -1;
    String fields[12]; // GPRMC has ~12 fields

    for (int i = 0; i < sentence.length(); i++) {
      if (sentence.charAt(i) == ',') {
        fields[fieldIndex++] = sentence.substring(lastComma + 1, i);
        lastComma = i;
      }
    }

    // Extract relevant fields
    String timeUTC = fields[1];         // hhmmss.ss
    String status = fields[2];          // A = valid
    String lat = fields[3];             // ddmm.mmmm
    String latDir = fields[4];          // N/S
    String lon = fields[5];             // dddmm.mmmm
    String lonDir = fields[6];          // E/W

    if (status == "A") {
      String formattedTime = formatTime(timeUTC);
      String formattedLat = convertToDecimal(lat, latDir);
      String formattedLon = convertToDecimal(lon, lonDir);

      Serial.print(formattedTime); Serial.print(",");
      Serial.print(formattedLat); Serial.print(",");
      Serial.println(formattedLon);
    }
  }
}

String formatTime(String rawTime) {
  if (rawTime.length() < 6) return "Invalid";
  String hh = rawTime.substring(0, 2);
  String mm = rawTime.substring(2, 4);
  String ss = rawTime.substring(4, 6);
  return hh + ":" + mm + ":" + ss;
}

String convertToDecimal(String rawCoord, String direction) {
  if (rawCoord.length() < 4) return "Invalid";

  int degLength = (rawCoord.indexOf('.') > 4) ? 3 : 2;
  float degrees = rawCoord.substring(0, degLength).toFloat();
  float minutes = rawCoord.substring(degLength).toFloat();
  float decimal = degrees + (minutes / 60.0);

  if (direction == "S" || direction == "W") decimal *= -1;
  return String(decimal, 6);
}
