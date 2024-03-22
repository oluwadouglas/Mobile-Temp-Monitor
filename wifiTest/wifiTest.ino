#include <twilio.hpp>
#include <ESP_Mail_Client.h>
#include <ThingSpeak.h>

#include <WiFi.h>
#include <DHT.h>
#include <HTTPClient.h>

// Values from Twilio (find them on the dashboard)
static const char *account_sid = "AC05773789aa3907ca3e532916944a750c";
static const char *auth_token = "ff7619bdba1ffc687516a24101f77cc0";
// Phone number should start with "+<countrycode>"
static const char *from_number = "+17625852476";

// You choose!
// Phone number should start with "+<countrycode>"
static const char *to_number = "+256787270058";
//static const char *message = "Hello from my ESP32 (via twilio)";

Twilio *twilio;

unsigned long myChannelNumber = 1738209;
const char *myWriteAPIKey = "Y6ELT24I315ZSKF9";
long randNumber;


/** The smtp host name e.g. smtp.gmail.com for GMail or smtp.office365.com for Outlook or smtp.mail.yahoo.com */
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "kinyonyidavid@gmail.com"
#define AUTHOR_PASSWORD ""

/* Recipient's email*/
#define RECIPIENT_EMAIL "kinyonyidavid@gmail.com"

/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

// Replace with your network credentials (STATION)
//Engdave's MacBook Pro, engdave1
const char *ssid = "kinyonyi david hope’s iPhone";
const char *password = "dave@1994";

unsigned long previousMillis = 0;
unsigned long interval = 30000;

WiFiClient client;  //for emmail sending

#define DHTPIN 4       // Digital pin connected to the DHT SENSOR
#define DHTTYPE DHT11  // DHT 11
//define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  twilio = new Twilio(account_sid, auth_token);
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
  //STARTING THE DHT SENSOR ---
  Serial.println(F("DHTxx test!"));
  dht.begin();
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  delay(5000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.println("");
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.println("");
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.

  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (x == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  if(t > 27){
    mailMessage("Hello Incharge, The Temperature in the room is too HIGH, please Take Imediate Action!");
    textMesssage("Hello Incharge, The Temperature in the room is too HIGH, please Take Imediate Action!");
  }
  if(t < 18){
    mailMessage("Hello Incharge, The Temperature in the room is too LOW, please Take Imediate Action!");
    textMesssage("Hello Incharge, The Temperature in the room is too LOW, please Take Imediate Action!");
  }
}

void textMesssage(String message) {
  //sendint text message
  String response;
  bool success = twilio->send_message(to_number, from_number, message, response);
  if (success) {
    Serial.println("Sent message successfully!");
  } else {
    Serial.println(response);
  }
  //end sending text message
}

void mailMessage(String textMsg) {
  /*  Set the network reconnection option */
  MailClient.networkReconnect(true);

  /** Enable the debug via Serial port
   * 0 for no debugging
   * 1 for basic level debugging
   *
   * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
   */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the Session_Config for user defined session credentials */
  Session_Config config;

  /* Set the session config */
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  /*
  Set the NTP config time
  For times east of the Prime Meridian use 0-12
  For times west of the Prime Meridian add 12 to the offset.
  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = F("ESP");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F("ESP Test Email");
  message.addRecipient(F("Sara"), RECIPIENT_EMAIL);

  /*Send HTML message*/
  /*String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Hello World!</h1><p>- Sent from ESP board</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;*/


  //Send raw text message
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;


  /* Connect to the server */
  if (!smtp.connect(&config)) {
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()) {
    Serial.println("\nNot yet logged in.");
  } else {
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status) {
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()) {
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}