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
int memory[10];
int turn = 1, memory_score = 0;

bool SW1_detect, SW2_detect, SW3_detect, SW4_detect;

bool onoff_detect;

bool starting = 0, mem_starting = 0, rec_starting = 0;

int delay_time;

int ending1 = 4, ending2 = 5, ending3 = 6, ending4 = 7;
int ending, ending_control;

int game_choice = 0;

bool wrong = 0;

int stringStart = 0;
int stringStop = 0;
int scrollCursor = 0;

int milisec = 0, sec = 0, target_sec = 10, print_sec;
int rec_milisec = 0, milisec_sum = 0, milisec_avr;
int rec_score = 0, rec_count = 0;
int rec_tone = 0;

bool rec_result = 0;

int mem_over = 0, rec_over = 0;

int lcd_shift_delay = 0;

bool lcd_clear = 0;

String line_choiceLevel = "  Choose LEVEL  ";
String line_levelExplain = "RD-level 1 / YE-level 2 / GN-level 3 / BU-level 4";
String line_clear1 = " level 1 Clear!";
String line_clear2 = " level 2 Clear!";
String line_clear3 = " level 3 Clear!";
String line_clear4 = " level 4 Clear!";
String line_over1 = " level 1 Over! ";
String line_over2 = " level 2 Over! ";
String line_over3 = " level 3 Over! ";
String line_over4 = " level 4 Over! ";
String line_choice1 = " LEVEL 1 Choice  ";
String line_choice2 = " LEVEL 2 Choice  ";
String line_choice3 = " LEVEL 3 Choice  ";
String line_choice4 = " LEVEL 4 Choice  ";
String line_end = "RD-Retry / YE-Choose Level / GN-Next Level / BU-Choose Game";
String line_choiceGame = "  Choose  GAME  ";
String line_gameExplain = "RD-MEMORY GAME / YE-REACTION SPEED GAME";
String line_Retry = "     RETRY!     ";
String line_nextLevel = "   Next Level   ";

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

// 게임 동작 중 한 라운드를 통과하였을 때, 부저 동작 패턴을 설정한 함수
void round_pass() {
  tone(BUZZER, 523);
  digitalWrite(LED1, HIGH);
  delay(100);
  tone(BUZZER, 659);
  digitalWrite(LED2, HIGH);
  delay(100);
  tone(BUZZER, 784);
  digitalWrite(LED3, HIGH);
  delay(100);
  tone(BUZZER, 988);
  digitalWrite(LED4, HIGH);
  delay(100);
  noTone(BUZZER);
  led_off();
  delay(1000);
}

// lcd 첫째 줄 고정, 둘째 줄을 왼쪽으로 한 칸씩 밀리도로 하는 함수
void lineShift(String line1, String line2) {

  lcd.setCursor(scrollCursor, 1);
  lcd.print(line2.substring(stringStart, stringStop));
  lcd.setCursor(0, 0);
  lcd.print(line1);

  if (lcd_shift_delay >= 300) {
    lcd.clear();

    if (stringStart == 0 && scrollCursor > 0) {
      scrollCursor--;
      stringStop++;
    } else if (stringStart == stringStop) {
      stringStart = stringStop = 0;
      scrollCursor = 16;
    } else if (stringStop == line2.length() && scrollCursor == 0) {
      stringStart++;
    } else {
      stringStart++;
      stringStop++;
    }
    lcd_shift_delay = 0;
  }
}

// 현재 레벨에 따라 lcd가 출력되도록 설정하는 함수
void befo_round() {
  lcd.setCursor(0, 1);
  if (ending == ending1)
    lcd.print(line_choice1);   // line_choice1 = " LEVEL 1 Choice  "
  else if (ending == ending2)
    lcd.print(line_choice2);   // line_choice2 = " LEVEL 2 Choice  "
  else if (ending == ending3)
    lcd.print(line_choice3);   // line_choice3 = " LEVEL 3 Choice  "
  else if (ending == ending4)
    lcd.print(line_choice4);   // line_choice4 = " LEVEL 4 Choice  "
}

// 게임 오버 혹은, 게임 클리어 했을 시, 나오는 선택 상황에서 동일 레벨을 다시 시도하는 함수
void retry() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line_Retry);      // line_Retry = "     RETRY!     "
  befo_round();
  ending_control = 5;
  memory_score = 0;
  wrong = 0;
  led_off();
  digitalWrite(LED1, HIGH);
  delay(1000);
  led_off();
  delay(1000);
}

// 게임 오버 혹은, 게임 클리어 했을 시, 나오는 선택 상황에서 레벨 선택창을 띄우는 함수
void level_choice() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line_choiceLevel);  // line_choiceLevel = "  Choose LEVEL  "
  ending_control = 0;
  memory_score = 0;
  wrong = 0;
  led_off();
  digitalWrite(LED2, HIGH);
  delay(1000);
  led_off();
  delay(1000);
}

// 게임 오버 혹은, 게임 클리어 했을 시, 나오는 선택 상황에서 다음 레벨을 시도하는 함수
void next_level() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line_nextLevel);  // line_nextLevel = "   Next Level   "
  if (ending == ending1)
    ending = ending2;
  else if (ending == ending2)
    ending = ending3;
  else if (ending == ending3)
    ending = ending4;
  befo_round();
  memory_score = 0;
  wrong = 0;
  led_off();
  digitalWrite(LED3, HIGH);
  delay(1000);
  led_off();
  delay(1000);
}

// 게임 오버 혹은, 게임 클리어 했을 시, 나오는 선택 상황에서 게임 선택창을 띄우는 함수
void game_rechoice() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line_choiceGame);   // line_choiceGame = "  Choose  GAME  "
  game_choice = 0;
  ending_control = 0;
  memory_score = 0;
  wrong = 0;
  led_off();
  digitalWrite(LED4, HIGH);
  delay(1000);
  led_off();
  delay(1000);
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
  lcd_shift_delay++;
  mem_over++;
  rec_over++;
  rec_tone++;

  if (milisec >= 1000) {
    sec++;
    milisec = 0;
  }
}

// 메모리 게임 함수
void memory_game() {
  if (mem_starting == 0) {  // 초기화면에서 메모리게임으로 넘어갈 때 한 번 동작
    lcd.setCursor(0, 0);
    lcd.print("  Choose  GAME  ");
    lcd.setCursor(0, 1);
    lcd.print("  MEMORY  GAME  ");
    digitalWrite(LED2, LOW);
    delay(1000);
    digitalWrite(LED1, LOW);
    delay(1000);
    mem_starting = 1;
  }

  lcd.clear();
  stringStart = 0;
  stringStop = 0;
  scrollCursor = 0;

  while (ending_control == 0) { // 레벨 설정 안되었을 때 동작
    sw_detect();                // on/off 버튼이 눌리면 모든 동작을 종료하고 초기 동작을 할 수 있도록 반복문을 나가는 동작
    if (onoff_detect == LOW) {
      lcd.clear();
      break;
    }

    led_on();
    lineShift(line_choiceLevel, line_levelExplain);

    if (SW1_detect == HIGH)       // 각 버튼에 맞는 레벨을 선택
      ending_control = 1;
    else if (SW2_detect == HIGH)
      ending_control = 2;
    else if (SW3_detect == HIGH)
      ending_control = 3;
    else if (SW4_detect == HIGH)
      ending_control = 4;
    else
      ending_control = 0;

    lcd.setCursor(0, 0);
    lcd.print(line_choiceLevel);
    switch (ending_control) {   // 라운드 수를 ending 이라는 변수에 endingn 이라는 레벨에 해당하는 변수를 저장한다.
      case 1:
        ending = ending1;
        led_off();
        digitalWrite(LED1, HIGH);
        lcd.setCursor(0, 1);
        lcd.print(" LEVEL 1 Choice  ");
        delay(1000);
        break;
      case 2:
        ending = ending2;
        led_off();
        digitalWrite(LED2, HIGH);
        lcd.setCursor(0, 1);
        lcd.print(" LEVEL 2 Choice  ");
        delay(1000);
        break;
      case 3:
        ending = ending3;
        led_off();
        digitalWrite(LED3, HIGH);
        lcd.setCursor(0, 1);
        lcd.print(" LEVEL 3 Choice  ");
        delay(1000);
        break;
      case 4:
        ending = ending4;
        led_off();
        digitalWrite(LED4, HIGH);
        lcd.setCursor(0, 1);
        lcd.print(" LEVEL 4 Choice  ");
        delay(1000);
        break;
    }
  }

  if (ending_control > 0 && ending_control < 5) { // 위 코드가 반복되지 않도록 레벨이 선택된다면, ending_control 변수를 5로 세팅.
    delay(1000);
    led_off();
    delay(1000);
    ending_control = 5;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  MEMORY  GAME  ");
  lcd.setCursor(0, 1);

  if (ending == ending1) {
    lcd.print("    LEVEL 1     ");
  } else if (ending == ending2) {
    lcd.print("    LEVEL 2     ");
  } else if (ending == ending3) {
    lcd.print("    LEVEL 3     ");
  } else if (ending == ending4) {
    lcd.print("    LEVEL 4     ");
  }

  if (memory_score >= 3) {    // memory_score 변수가 3이상이라면 즉, 하나의 레벨을 모두 마쳤다면 아래 LED, 부저 동작을 수행
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

    stringStart = 0;
    stringStop = 0;
    scrollCursor = 0;

    while (memory_score == 3) {   // memory_score 변수가 3이라면, 무한반복
      sw_detect();                // on/off 버튼이 눌리면 모든 동작을 종료하고 초기 동작을 할 수 있도록 반복문을 나가는 동작
      if (onoff_detect == LOW) {
        lcd.clear();
        break;
      }

      // 레벨에 따라 첫째 줄에 ~레벨 클리어했다는 문구와, 둘째 줄에 옵션 선택 문구를 출력
      if (ending == ending1) lineShift(line_clear1, line_end);
      else if (ending == ending2) lineShift(line_clear2, line_end);
      else if (ending == ending3) lineShift(line_clear3, line_end);
      else if (ending == ending4) lineShift(line_clear4, line_end);

      // 둘째 줄에 안내된 사항에 따라, 빨 / 노 / 초 / 파 순으로 각각 재시도, 레벨 선택, 다음 레벨, 게임 선택 기능을 하며 그에 맞는 함수를 호출
      if (SW1_detect == HIGH) {
        retry();
        break;
      } else if (SW2_detect == HIGH) {
        level_choice();
        break;
      } else if (SW3_detect == HIGH) {
        next_level();
        break;
      } else if (SW4_detect == HIGH) {
        game_rechoice();
        break;
      }
    }
  } else if (memory_score < 3) {    // memory_score 변수가 3 미만일 때, 즉 레벨 클리어하기 전이라면,
    Serial.print("ending : ");
    Serial.println(ending);

    if (ending == ending1) {        // 레벨에 따라 그에 맞는 delay_time 변수를 수정하며, 이는 게임 중 속도를 의미.
      delay_time = 400;             // ms 단위이며, 100ms씩 빨라진다
    } else if (ending == ending2) {
      delay_time = 300;
    } else if (ending == ending3) {
      delay_time = 200;
    } else if (ending == ending4) {
      delay_time = 100;
    }

    // 레벨마다 지정된 라운드만큼 반복하여 LED 패턴을 제시
    for (int i = 0; i < ending; i++) {
      // on/off 버튼이 눌리면 모든 동작을 종료하고 초기 동작을 할 수 있도록 반복문을 나가는 동작
      sw_detect();
      if (onoff_detect == LOW) {
        lcd.clear();
        break;
      }

      Serial.print("i: ");
      Serial.println(i);

      // 1~4의 랜덤한 숫자값을 받아와 LED를 출력
      randomled = random(1, 5);
      memory[i + 1] = randomled;

      switch (memory[i + 1]) {
        case 1:
          digitalWrite(LED1, HIGH);
          tone(BUZZER, 523);
          Serial.println("turn 1 test");
          break;
        case 2:
          digitalWrite(LED2, HIGH);
          tone(BUZZER, 659);
          Serial.println("turn 2 test");
          break;
        case 3:
          digitalWrite(LED3, HIGH);
          tone(BUZZER, 784);
          Serial.println("turn 3 test");
          break;
        case 4:
          digitalWrite(LED4, HIGH);
          tone(BUZZER, 988);
          Serial.println("turn 4 test");
          break;
      }

      right();

      Serial.print("delay_time : ");
      Serial.println(delay_time);
    }

    mem_over = 0;

    // 앞서 공개한 패턴을 똑같이 입력해야하는 구간
    for (int j = 0; j < ending;) {

      Serial.print("ending : ");
      Serial.println(ending);
      Serial.print("j : ");
      Serial.println(j);

      // on/off 버튼이 눌리면 모든 동작을 종료하고 초기 동작을 할 수 있도록 반복문을 나가는 동작
      sw_detect();
      if (onoff_detect == LOW) {
        lcd.clear();
        break;
      }

      // 감지된 버튼과 켜져있는 LED의 위치가 같다면 ex) 2번 스위치 ON & 2번 LED(YELLOW) ON
      // 누른 버튼의 LED, 부저를 일정시간 ON하여 맞게 누름을 인지시킴
      if (SW1_detect == LOW && SW2_detect == LOW && SW3_detect == LOW && SW4_detect == LOW) {
        //Serial.println("no click");
      } else if (SW1_detect == HIGH && memory[j + 1] == 1) { // HIGH가 CLOSE(연결)이라 가정.
        mem_over = 0;
        digitalWrite(LED1, HIGH);
        tone(BUZZER, 523);
        j++;
        right();
      } else if (SW2_detect == HIGH && memory[j + 1] == 2) {
        mem_over = 0;
        digitalWrite(LED2, HIGH);
        tone(BUZZER, 659);
        j++;
        right();
      } else if (SW3_detect == HIGH && memory[j + 1] == 3) {
        mem_over = 0;
        digitalWrite(LED3, HIGH);
        tone(BUZZER, 784);
        j++;
        right();
      } else if (SW4_detect == HIGH && memory[j + 1] == 4) {
        mem_over = 0;
        digitalWrite(LED4, HIGH);
        tone(BUZZER, 988);
        j++;
        right();
      } else {
        reset();
        delay(3000);
        memory_score = -1;
        wrong = 1;
        mem_over = 0;
        break; // continue break 차이 확인 및 break 추가한 상태에서 추가 테스트
      }

      // 타이머 인터럽트에서 1씩 증가한 mem_over 변수가 2000이 넘으면, 타임 오버로 게임 종료
      if (mem_over >= 2000) {
        reset();
        memory_score = -1;
        wrong = 1;
        delay(3000);
        mem_over = 0;
        break; // continue break 차이 확인 및 break 추가한 상태에서 추가 테스트
      }
    }

    // 두 번째 for문에서 j값을 채워 반복문을 빠져나오면, memory_score 변수를 1씩 증가
    memory_score++;

    // memory_score 변수가 1이거나, 2일 때 (라운드를 통과해 다음 라운드로 넘어갈 때), round_pass 함수를 사용해 특정 패턴을 출력
    if (memory_score == 1 || memory_score == 2) round_pass();
    
  }

  Serial.print("wrong : ");
  Serial.println(wrong);

  if (wrong == 1) {     // 잘못된 버튼을 누렀을 때
    stringStart = 0;
    stringStop = 0;
    scrollCursor = 0;

    led_on();

    while (wrong == 1) {
      sw_detect();
      if (onoff_detect == LOW) {
        lcd.clear();
        break;
      }

      if (ending == ending1) lineShift(line_over1, line_end);
      else if (ending == ending2) lineShift(line_over2, line_end);
      else if (ending == ending3) lineShift(line_over3, line_end);
      else if (ending == ending4) lineShift(line_over4, line_end);

      if (SW1_detect == HIGH) {
        retry();
        break;
      } else if (SW2_detect == HIGH) {
        level_choice();
        break;
      } else if (SW3_detect == HIGH) {
        next_level();
        break;
      } else if (SW4_detect == HIGH) {
        game_rechoice();
        break;
      }
    }
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
        game_rechoice();
        rec_result = 0;
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
  lcd.setCursor(6, 1);
  lcd.print("GAME");
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
    if (game_choice == 0) { // 아무 게임이 정해지지 않았을 경우, 그에 맞는 LCD를 띄우며, 빨간 색 스위치를 누르면
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, HIGH);
      lineShift(line_choiceGame, line_gameExplain);
      if (SW1_detect == HIGH) {
        mem_starting = 0;
        game_choice = 1;
      } else if (SW2_detect == HIGH) {
        rec_starting = 0;
        game_choice = 2;
      }
    } else if (game_choice == 1)
      memory_game();
    else if (game_choice == 2)
      reaction_speed();
  } else if (onoff_detect == LOW) {
    if (lcd_clear == LOW) {
      lcd.clear();
      lcd_clear = HIGH;
    }
    noTone(BUZZER);
    lcd.setCursor(4, 0);
    lcd.print("ELEPARTS");
    lcd.setCursor(6, 1);
    lcd.print("GAME");
    led_off();
    starting = 0;
    ending_control = 0;
    wrong = 0;
    game_choice = 0;
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
