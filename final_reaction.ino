#define onoff 2                         // 사용 핀에 이름 지정
#define BUZZER 3
#define SW1 4
#define LED1 5
#define SW2 6
#define LED2 7
#define SW3 8
#define LED3 9
#define SW4 10
#define LED4 11

#include <Wire.h>                       // 1602 LCD 라이브러리 세팅
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <FspTimer.h>                   // Arduino UNO R4 Timer Interrupt 라이브러리 설정

static FspTimer fsp_timer;

// 사용할 변수, 문자열 등 선언
int randomled;

bool SW1_detect, SW2_detect, SW3_detect, SW4_detect;

bool onoff_detect;

bool starting = 0, rec_starting = 0;

int delay_time;

int milisec = 0, sec = 0, target_sec = 10, print_sec;
int rec_milisec = 0, milisec_sum = 0, milisec_avr;
int rec_score = 0, rec_count = 0;
int rec_tone = 0;

bool rec_result = 0;

int rec_over = 0;

bool lcd_clear = 0;

// 모든 돔형 스위치의 LED를 키는 함수
void led_on() {
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
}

// 모든 돔형 스위치의 LED를 끄는 함수
void led_off() {
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
}

// 메모리 게임 동작 중 실행중인 LED와 부저를 끄고 다음으로 넘어갈 때, 사용하기 위한 함수
void right() {
  delay(delay_time);
  led_off();
  noTone(BUZZER);
  delay(delay_time);
}

// 전체 동작의 초기 동작과 게임 오버 시 종료 동작을 하기 위한 함수
void reset() {
  for (int k = 0; k < 3; k++) { // k = 0; - 변수 초기화 / k < 3; - k 변수가 3보다 작을 경우, 중괄호 안의 코드를 수행 / k++ - k 변수에 1을 더함
    sw_detect();                // on/off 버튼이 눌리면 모든 동작을 종료하고 초기 동작을 할 수 있도록 반복문을 나가는 동작
    if (onoff_detect == LOW) {
      break;
    }

    led_on();
    tone(BUZZER, 247);
    delay(1000);
    led_off();
    noTone(BUZZER);
    delay(1000);
  }
}

// on/off 스위치와 4개의 돔형 스위치의 입력 값을 받아오는 함수
void sw_detect() {
  onoff_detect = digitalRead(onoff);
  SW1_detect = digitalRead(SW1);
  SW2_detect = digitalRead(SW2);
  SW3_detect = digitalRead(SW3);
  SW4_detect = digitalRead(SW4);
}

// 반응속도 게임 중 1~4 중 랜덤한 숫자를 받아와 해당하는 LED를 켜, 누를 버튼을 제시합니다.
void ran_led() {
  randomled = random(1, 4);
  Serial.print("randomled : ");
  Serial.print(randomled);

  led_off();
  if (randomled == 1)
    digitalWrite(LED1, HIGH);
  else if (randomled == 2)
    digitalWrite(LED2, HIGH);
  else if (randomled == 3)
    digitalWrite(LED3, HIGH);
  else if (randomled == 4)
    digitalWrite(LED4, HIGH);
}

// 반응속도 게임 동작 중 버튼을 눌렀을 때, 눌러야할 버튼을 눌렀다면 점수 증가, 잘못 눌렀다면 점수 감소하는 함수
void rec_button(bool right) {
  if (right == HIGH) {
    rec_score++;
    tone(BUZZER, 784);
  } else if (right == LOW) {
    rec_score--;
    tone(BUZZER, 247);
  }
  rec_count++;
  milisec_sum = milisec_sum + rec_milisec;    // milisec 측정 값을 총합해 동작이 끝났을 때 rec_count로 나눠 버튼 반응속도 값을 계산합니다.
  rec_milisec = 0;
  lcd.print(rec_score);
  rec_tone = 0;
}

// 타이머 인터럽트 동작 - 1ms에 한번 동작
void timer_callback([[maybe_unused]]timer_callback_args_t *arg) {
  milisec++;
  rec_milisec++;
  rec_over++;
  rec_tone++;

  if (milisec >= 1000) {
    sec++;
    milisec = 0;
  }
}

// 반응속도 게임 함수
void reaction_speed() {
  if (rec_starting == 0) {
    digitalWrite(LED1, LOW);
    delay(1000);
    digitalWrite(LED2, LOW);

    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("READY!");
    delay(1000);

    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("SET!");
    delay(1000);

    lcd.clear();
    lcd.setCursor(7, 0);
    lcd.print("GO!");
    delay(1000);

    lcd.clear();
    rec_over = 0;
    rec_score = 0;
    rec_count = 0;
    rec_milisec = 0;
    milisec_sum = 0;
    milisec = 0;
    sec = 0;

    ran_led();

    rec_starting = 1;
  }

  sw_detect();

  print_sec = target_sec - sec;

  lcd.setCursor(0, 0);
  lcd.print("sec : ");

  if (print_sec > 0) {
    lcd.setCursor(6, 0);
    lcd.print(print_sec);
    if (print_sec >= 10 || print_sec < 0) {
      lcd.setCursor(8, 0);
      lcd.print("s");
    } else if (print_sec < 10 && print_sec >= 0) {
      lcd.setCursor(7, 0);
      lcd.print("s ");
    }
  } else if (print_sec <= 0) {
    lcd.setCursor(6, 0);
    lcd.print("TIME OVER");

    delay(1000);

    lcd.clear();
    rec_result = 1;

  }

  Serial.print("target_sec : ");
  Serial.println(target_sec);
  Serial.print("sec : ");
  Serial.println(sec);

  lcd.setCursor(0, 1);
  lcd.print("Score : ");
  lcd.setCursor(8, 1);
  lcd.print(rec_score);

  if (print_sec > 0) {

    if (rec_over < 3000) {
      if (SW1_detect == HIGH || SW2_detect == HIGH || SW3_detect == HIGH || SW4_detect == HIGH) {
        rec_over = 0;
      }
    } else if (rec_over >= 3000) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   GAME OVER   ");

      delay(1000);

      lcd.clear();
      rec_result = 1;
    }


    lcd.setCursor(8, 1);
    if (SW1_detect == HIGH && digitalRead(LED1) == HIGH) {
      rec_button(HIGH);
      led_off();
      ran_led();
    } else if (SW1_detect == HIGH && (digitalRead(LED2) == HIGH || digitalRead(LED3) == HIGH || digitalRead(LED4) == HIGH)) {
      rec_button(LOW);
    } else if (SW2_detect == HIGH && digitalRead(LED2) == HIGH) {
      rec_button(HIGH);
      led_off();
      ran_led();
    } else if (SW2_detect == HIGH && (digitalRead(LED1) == HIGH || digitalRead(LED3) == HIGH || digitalRead(LED4) == HIGH)) {

      // 게임 오버 혹은, 게임 클리어 했을 시, 나오는 선택 상황에서 레벨 선택창을 띄우는 함수LOW);
    } else if (SW3_detect == HIGH && digitalRead(LED3) == HIGH) {
      rec_button(HIGH);
      led_off();
      ran_led();
    } else if (SW3_detect == HIGH && (digitalRead(LED1) == HIGH || digitalRead(LED2) == HIGH || digitalRead(LED4) == HIGH)) {
      rec_button(LOW);
    } else if (SW4_detect == HIGH && digitalRead(LED4) == HIGH) {
      rec_button(HIGH);
      led_off();
      ran_led();
    } else if (SW4_detect == HIGH && (digitalRead(LED1) == HIGH || digitalRead(LED2) == HIGH || digitalRead(LED3) == HIGH)) {
      rec_button(LOW);
    }

    if (rec_tone >= 100) {
      noTone(BUZZER);
    }
  }

  if (rec_result == 1) {

    milisec_avr = milisec_sum / rec_count;

    lcd.setCursor(0, 0);
    lcd.print("Score : ");
    lcd.setCursor(8, 0);
    lcd.print(rec_score);
    lcd.setCursor(0, 1);
    lcd.print("Speed : ");
    lcd.setCursor(8, 1);
    lcd.print(milisec_avr);
    if (milisec_avr < 10) {
      lcd.setCursor(9, 1);
      lcd.print("ms");
    } else if (milisec_avr >= 10 && milisec_avr < 100) {
      lcd.setCursor(10, 1);
      lcd.print("ms");
    } else if (milisec_avr >= 100 && milisec_avr < 1000) {
      lcd.setCursor(11, 1);
      lcd.print("ms");
    } else if (milisec_avr >= 1000) {
      lcd.setCursor(12, 1);
      lcd.print("ms");
    }
    led_on();

    tone(BUZZER, 523);
    digitalWrite(LED1, HIGH);
    delay(500);
    tone(BUZZER, 659);
    digitalWrite(LED2, HIGH);
    delay(500);
    tone(BUZZER, 784);
    digitalWrite(LED3, HIGH);
    delay(500);
    tone(BUZZER, 988);
    digitalWrite(LED4, HIGH);
    delay(500);
    noTone(BUZZER);

    while (rec_result == 1) {
      sw_detect();
      if (onoff_detect == LOW) {
        lcd.clear();
        break;
      }

      if (SW1_detect == HIGH || SW2_detect == HIGH || SW3_detect == HIGH || SW4_detect == HIGH) {
        rec_result = 0;
        rec_starting = 0;
      }

    }
  }

  sw_detect();
  if (onoff_detect == LOW) lcd.clear();
}

// 한 번 동작하는 setup 함수
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // 사용 스위치, LED, 부저의 핀번호 설정
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
  pinMode(SW4, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(onoff, INPUT);

  // 타이머 인터럽트 실행
  uint8_t timer_type;
  int8_t timer_ch = FspTimer::get_available_timer(timer_type);
  if (timer_ch < 0) {
    Serial.println("get_available_timer() failed.");
    return;
  }
  fsp_timer.begin(TIMER_MODE_PERIODIC, timer_type, static_cast<uint8_t>(timer_ch), 1000.0, 25.0, timer_callback, nullptr);
  fsp_timer.setup_overflow_irq();
  fsp_timer.open();
  fsp_timer.start();
  Serial.println("started.");

  // LCD 초기화
  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(4, 0);
  lcd.print("ELEPARTS");
  lcd.setCursor(1, 1);
  lcd.print("REACTION  GAME");
}

// 반복 동작하는 함수
void loop() {
  // put your main code here, to run repeatedly:
  sw_detect();
  Serial.print("onoff 스위치 : ");
  Serial.println(onoff_detect);
  if (onoff_detect == HIGH) {
    lcd_clear = LOW;
    if (starting == 0) {    // ON/OFF 스위치를 ON이 되게 눌렀을 경우, 한 번 중괄호 내 동작이 실행
      reset();
      delay(3000);
      starting = 1;
    }
    reaction_speed();
  } else if (onoff_detect == LOW) {
    if (lcd_clear == LOW) {
      lcd.clear();
      lcd_clear = HIGH;
    }
    noTone(BUZZER);
    lcd.setCursor(4, 0);
    lcd.print("ELEPARTS");
    lcd.setCursor(1, 1);
    lcd.print("REACTION  GAME");
    led_off();
    starting = 0;
    rec_starting = 0;
  }

  Serial.print("RED : ");
  Serial.println(SW1_detect);
  Serial.print("YELLOW : ");
  Serial.println(SW2_detect);
  Serial.print("GREEN : ");
  Serial.println(SW3_detect);
  Serial.print("BLUE : ");
  Serial.println(SW4_detect);
}
