#include <SPI.h>
#include <TMCStepper.h>
#include <cppQueue.h>
#include <strings.h>

#define SERIAL_RX_BUFFER_SIZE 2048

#define CS_PIN PB12  // Chip select
#define SW_MOSI PB15 // Software Master Out Slave In (MOSI)
#define SW_MISO PB14 // Software Master In Slave Out (MISO)
#define SW_SCK PB13  // Software Slave Clock (SCK)

#define CHANNEL_1_ADC_PIN PA0
#define CHANNEL_2_ADC_PIN PA1
#define CHANNEL_3_ADC_PIN PA2
#define CHANNEL_4_ADC_PIN PA3
#define CHANNEL_5_ADC_PIN PA4
#define CHANNEL_6_ADC_PIN PA5
#define CHANNEL_7_ADC_PIN PA6
#define CHANNEL_8_ADC_PIN PA7

#define CHANNEL_1_PWM_PIN PB9
#define CHANNEL_2_PWM_PIN PB8
#define CHANNEL_3_PWM_PIN PB7
#define CHANNEL_4_PWM_PIN PB6
#define CHANNEL_5_PWM_PIN PA10
#define CHANNEL_6_PWM_PIN PA9
#define CHANNEL_7_PWM_PIN PA8
#define CHANNEL_8_PWM_PIN PB1

const uint8_t ADC_PINS[8] = {
    CHANNEL_1_ADC_PIN, CHANNEL_2_ADC_PIN, CHANNEL_3_ADC_PIN, CHANNEL_4_ADC_PIN,
    CHANNEL_5_ADC_PIN, CHANNEL_6_ADC_PIN, CHANNEL_7_ADC_PIN, CHANNEL_8_ADC_PIN};
const uint8_t PWM_PINS[8] = {
    CHANNEL_1_PWM_PIN, CHANNEL_2_PWM_PIN, CHANNEL_3_PWM_PIN, CHANNEL_4_PWM_PIN,
    CHANNEL_5_PWM_PIN, CHANNEL_6_PWM_PIN, CHANNEL_7_PWM_PIN, CHANNEL_8_PWM_PIN};

// Stepper Motor Driver
TMC5130Stepper driver(CS_PIN, SW_MOSI, SW_MISO, SW_SCK);

// Command Word Definitions
enum CommandWord {
  COMMAND_HOME,
  COMMAND_MOVE_ABS,
  COMMAND_SET_ROW_POS,
  COMMAND_SET_LED_PWR,
  COMMAND_SCAN_WELL,
  COMMAND_SCAN_ALL,
};

typedef struct {
  int32_t row_positions[12];
  uint8_t led_powers[8];
  int32_t position;
  CommandWord command_word;
  uint8_t row;
  uint8_t col;
} Command;

bool string_to_command(String s, Command *c) {
  while (!s.startsWith("/") && s.length() != 0) {
    s.remove(0);
  };
  if (s.length() == 0) {
    return false;
  }

  auto index = s.indexOf(' ');
  auto command_word =
      (index == -1) ? s.substring(0, s.length()) : s.substring(0, index);
  if (command_word == "/echo") {
    Serial.print('@');
    Serial.print(s.substring(1));
    return false;
  } else if (command_word == "/home") {
    c->command_word = COMMAND_HOME;
  } else if (command_word == "/move_abs") {
    c->command_word = COMMAND_MOVE_ABS;
    c->position = s.substring(index + 1).toInt();
  } else if (command_word == "/set_row_pos") {
    c->command_word = COMMAND_SET_ROW_POS;
    sscanf(s.c_str() + index, "%li %li %li %li %li %li %li %li %li %li %li %li",
           &(c->row_positions[0]), &(c->row_positions[1]),
           &(c->row_positions[2]), &(c->row_positions[3]),
           &(c->row_positions[4]), &(c->row_positions[5]),
           &(c->row_positions[6]), &(c->row_positions[7]),
           &(c->row_positions[8]), &(c->row_positions[9]),
           &(c->row_positions[10]), &(c->row_positions[11]));
  } else if (command_word == "/set_led_pwr") {
    c->command_word = COMMAND_SET_LED_PWR;
    sscanf(s.c_str() + index, "%li %li %li %li %li %li %li %li",
           &(c->led_powers[0]), &(c->led_powers[1]), &(c->led_powers[2]),
           &(c->led_powers[3]), &(c->led_powers[4]), &(c->led_powers[5]),
           &(c->led_powers[6]), &(c->led_powers[7]));
  } else if (command_word == "/scan_well") {
    c->command_word = COMMAND_SCAN_WELL;
    sscanf(s.c_str() + index, "%d %d", &(c->row), &(c->col));
  } else if (command_word == "/scan_all") {
    c->command_word = COMMAND_SCAN_ALL;
  } else {
    return false;
  }
  return true;
}
cppQueue command_queue(sizeof(Command), 128, FIFO, true);

void setup() {
  // initialize serial:
  Serial.setTimeout(10);
  Serial.begin(9600);

  // initialize SPI CS Pin
  pinMode(CS_PIN, OUTPUT);

  driver.begin(); //  SPI: Init CS pins and possible SW SPI pins

  driver.CHOPCONF(0x000100C3);
  // driver.IHOLD_IRUN(0x00061F0A);
  driver.ihold(1);
  driver.irun(31);
  driver.TPOWERDOWN(0x0000000A);
  driver.GCONF(0x00000004);
  driver.TPWMTHRS(0x000001F4);
  driver.PWMCONF(0x000401C8);

  // driver.a1(1000);
  // driver.v1(50000);
  // driver.AMAX(500);
  // driver.VMAX(200000);
  // driver.DMAX(700);
  // driver.d1(1400);
  // driver.VSTOP(10);
  driver.a1(1000);
  driver.v1(10000);
  driver.AMAX(500);
  driver.VMAX(75000);
  driver.DMAX(700);
  driver.d1(1400);
  driver.VSTOP(10);

  driver.RAMPMODE(0);

  driver.en_softstop(false); // Hard Stop

  driver.stop_r_enable(true);
  driver.pol_stop_r(false); // Active High

  driver.stop_l_enable(true);
  driver.pol_stop_l(false);

  // initialize ADC pins
  analogReadResolution(12);
  for (auto i = 0; i < 8; ++i) {
    pinMode(ADC_PINS[i], INPUT);
  }

  // initialize PWM pins
  for (auto i = 0; i < 8; ++i) {
    pinMode(PWM_PINS[i], OUTPUT);
    analogWrite(PWM_PINS[i], 0);
  }
}

void loop() {
  static int32_t row_positions[12] = {
      51200 * 1, 51200 * 2, 51200 * 3, 51200 * 4,  51200 * 5,  51200 * 6,
      51200 * 7, 51200 * 8, 51200 * 9, 51200 * 10, 51200 * 11, 51200 * 12};
  static uint8_t led_powers[8] = {32 * 0, 32 * 1, 32 * 2, 32 * 3,
                                  32 * 4, 32 * 5, 32 * 6, 32 * 7};

  while (!command_queue.isEmpty()) {
    Command c;
    command_queue.pop(&c);
    switch (c.command_word) {
    case COMMAND_HOME: {
      driver.RAMPMODE(2); // Set to Negative Velocity
      while (!driver.event_stop_l())
        ;
      driver.XACTUAL(0);
      driver.XTARGET(0);
      driver.RAMPMODE(0); // Set to Position Mode
      Serial.println("@home");
      break;
    }
    case COMMAND_MOVE_ABS: {
      driver.XTARGET(c.position);
      while (1) {
        if (driver.position_reached()) {
          break;
        }
        if (driver.event_stop_l() || driver.event_stop_r()) {
          driver.XACTUAL(0);
          driver.XTARGET(0);
          break;
        }
      }
      Serial.println(String("@move_abs " + String(c.position)));
      break;
    }
    case COMMAND_SET_ROW_POS: {
      Serial.print("@set_row_pos");
      for (int i = 0; i < 12; ++i) {
        row_positions[i] = c.row_positions[i];
        Serial.print(' ');
        Serial.print(row_positions[i]);
      }
      Serial.println("");
      break;
    }
    case COMMAND_SET_LED_PWR: {
      Serial.print("@set_led_pwr");
      for (int i = 0; i < 8; ++i) {
        led_powers[i] = c.led_powers[i];
        Serial.print(' ');
        Serial.print(led_powers[i]);
      }
      Serial.println("");
      break;
    }
    case COMMAND_SCAN_WELL: {
      const auto row = c.row;
      const auto col = c.col;

      auto x_position = row_positions[row];
      auto led_intensity = led_powers[col];
      auto pwm_pin = PWM_PINS[col];

      driver.XTARGET(x_position);
      while (1) {
        serialEvent();
        if (driver.position_reached()) {
          break;
        }
        serialEvent();
        if (driver.event_stop_l() || driver.event_stop_r()) {
          driver.XACTUAL(0);
          driver.XTARGET(0);
          break;
        }
      }

      analogWrite(pwm_pin, led_intensity);
      for (auto i = 0; i < 500; ++i) {
        delayMicroseconds(100);
        serialEvent();
      }

      auto result = analogRead(ADC_PINS[col]);
      analogWrite(pwm_pin, 0);

      Serial.print("@scan_well ");
      Serial.print(row);
      Serial.print(" ");
      Serial.print(col);
      Serial.print(" ");
      Serial.println(result);
      break;
    }
    case COMMAND_SCAN_ALL:
      for (auto row_index = 0; row_index < 12; ++row_index) {
        for (auto col_index = 0; col_index < 8; ++col_index) {
          Command c;
          c.command_word = COMMAND_SCAN_WELL;
          c.row = row_index;
          c.col = col_index;
          command_queue.push(&c);
        }
      }
      break;
    default:
      break;
    }
  }
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  static String inputString;
  Command c;
  if (Serial.available()) {
    inputString += Serial.readStringUntil('\n');
    if (string_to_command(inputString, &c)) {
      command_queue.push(&c);
    } else {
      Serial.print("@error invalid command ");
      Serial.println(inputString);
    }
    inputString = "";
  }
}
