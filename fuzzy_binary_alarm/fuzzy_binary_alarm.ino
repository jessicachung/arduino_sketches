/*
 * Fuzzy Binary Alarm Clock
 * 
 * Timer for waking up that counts up to 8 hours, then buzzes twice. Once buzz 
 * at 8h and again at 8h10m.
 * 6 LEDs indicate the time the clock has been running. Four 5mm LEDs on the left
 * represent the hours. Two 3mm LEDs on the right represent the minutes in 
 * 15 minute increments.
 * Pressing the button increases the timer by 10 minutes. A long press decreases 
 * the timer by an hour.
 * 
 */

// Debug mode
//#define DEBUG

// Define pins
#define HOUR_8 3
#define HOUR_4 4
#define HOUR_2 5
#define HOUR_1 6
#define MIN_30 11
#define MIN_15 12
#define BUZZER 10
#define OFFSET_PIN 13

// Tone variables
unsigned int buzz_durations[] = {500, 1000};
int tone_frequency = 100;
int buzz_delay = 600;
int buzz_count = 0;
long last_buzz = 0;

// Time variables
long seconds;
int minutes;
int hours;

// Offset variables
int minute_increase = 10;
int minute_decrease = 60;
long second_offset = 0;
long press_time;
boolean offset = false;
boolean negative_offset = false;

void blink_minute_leds(int delay_ms) {
  digitalWrite(MIN_30, LOW);
  digitalWrite(MIN_15, HIGH);
  delay(delay_ms);
  digitalWrite(MIN_30, HIGH);
  digitalWrite(MIN_15, LOW);
  delay(delay_ms);
}

void setup() {
  #ifdef DEBUG
    Serial.begin(9600);
  #endif
  pinMode(HOUR_1, OUTPUT);
  pinMode(HOUR_2, OUTPUT);
  pinMode(HOUR_4, OUTPUT);
  pinMode(HOUR_8, OUTPUT);
  pinMode(MIN_15, OUTPUT);
  pinMode(MIN_30, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(OFFSET_PIN, INPUT);
  digitalWrite(HOUR_8, LOW);
}

void loop() {
  seconds = millis()/1000 + second_offset;
  minutes = (seconds / 60) % 60;
  hours = seconds / 3600;

  #ifdef DEBUG
    // Make each real-time second equal 15 minutes in debug mode
    seconds = millis()/1000 * 900 + second_offset;
    minutes = (seconds / 60) % 60;
    hours = seconds / 3600;
    Serial.print(hours);
    Serial.print(":");
    Serial.println(minutes);
  #endif

  // Detect if button is pressed
  while (digitalRead(OFFSET_PIN) == HIGH) {
    if (! offset) {
      tone(BUZZER, tone_frequency, 10);
      offset = true;
      press_time = millis();
    }
    // Detect if button is pressed for over 2 seconds
    if (! negative_offset & ((millis() - press_time) > 2000)) {
      tone(BUZZER, tone_frequency, 20);
      negative_offset = true;
    }
    delay(50);
  }

  // Offset time
  if (offset) {
    if (negative_offset) {
      second_offset -= minute_decrease * 60;
      #ifdef DEBUG
        Serial.print("Offset by -");
        Serial.print(minute_decrease);
        Serial.println(" minutes.");
      #endif
    } else {
      second_offset += minute_increase * 60;
      #ifdef DEBUG
        Serial.print("Offset by +");
        Serial.print(minute_increase);
        Serial.println(" minutes.");
      #endif
    }
    offset = false;
    negative_offset = false;
  }

  // LED 0100 00
  if (hours >=4) {
    digitalWrite(HOUR_4, HIGH);
  } else {
    digitalWrite(HOUR_4, LOW);
  }

  // LED 0010 00
  if (hours == 2 | hours == 3 | hours == 6 | hours == 7) {
    digitalWrite(HOUR_2, HIGH);
  } else {
    digitalWrite(HOUR_2, LOW);
  }

  // LED 0001 00
  if (hours == 1 | hours == 3 | hours == 5 | hours == 7) {
    digitalWrite(HOUR_1, HIGH);
  } else {
    digitalWrite(HOUR_1, LOW);
  }

  // LED 0000 10
  if (minutes >= 30) {
    digitalWrite(MIN_30, HIGH);
  } else {
    digitalWrite(MIN_30, LOW);
  }
  
  // LED 0000 01
  if ((minutes >= 15 & minutes < 30) | (minutes >= 45)) {
    digitalWrite(MIN_15, HIGH);
  } else {
    digitalWrite(MIN_15, LOW);
  }

  // All LEDs on after 8 hours
  if (hours >= 8) {
    digitalWrite(HOUR_8, HIGH);
    digitalWrite(HOUR_4, HIGH);
    digitalWrite(HOUR_2, HIGH);
    digitalWrite(HOUR_1, HIGH);

    // Buzz after 8 hours
    if ( ((seconds - last_buzz) > buzz_delay) & 
         (buzz_count < sizeof(buzz_durations)/sizeof(int)) ) {
      #ifdef DEBUG
         Serial.println("Buzzing!");
      #endif
      tone(BUZZER, tone_frequency, buzz_durations[buzz_count]);
      buzz_count++;
      last_buzz = seconds;
    }

    // After buzzing the set number of times, blink the minute LEDs continuously
    while (buzz_count >= sizeof(buzz_durations)/sizeof(int)) {
      blink_minute_leds(500);
    }
  }
  
  delay(1000);
}
