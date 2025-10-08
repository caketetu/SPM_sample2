//SPMスレーブデバイスのサンプル二つ目
//二つのLED制御アプリケーション
#include "MyModbus.h"
#define DEV_ID 0x30

#define LED_PIN 32

//UART Serial2(A4, A5);
MyModbus modbus;

int16_t MODEL_NO = 49;
int16_t VERSION = 1;
int16_t SUB_VERSION = 0;

int16_t dev_id;      //デバイスID
int16_t duty_max;    //最大Duty
int16_t duty_min;    //最小Duty
int16_t duty;        //Duty
int16_t interval=10;    //点滅インターバル
int16_t mode;        //動作モード
int16_t pattern[7];  //動作パターン
int16_t led_on = 0;
int16_t mode_cnt = 0;

int16_t na_adr;

uint8_t dio = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //modbus.id = dev_id;
  modbus.id = 48;
  //Serial2.begin(115200);

  //各レジスタ初期化
  //コイル---rwレジスタと同じ
  modbus.p_coils[0] = &dio;  //&rw_value[0];
  modbus.p_coils[1] = &dio;  //&rw_value[1];

  modbus.p_holding_regs[0] = &dev_id;
  modbus.p_holding_regs[1] = &duty_max;
  modbus.p_holding_regs[2] = &duty_min;
  modbus.p_holding_regs[3] = &duty;
  modbus.p_holding_regs[4] = &interval;
  modbus.p_holding_regs[5] = &mode;
  modbus.p_holding_regs[6] = &na_adr;
  modbus.p_holding_regs[7] = &na_adr;
  modbus.p_holding_regs[8] = &na_adr;
  modbus.p_holding_regs[9] = &pattern[0];
  modbus.p_holding_regs[10] = &pattern[1];
  modbus.p_holding_regs[11] = &pattern[2];
  modbus.p_holding_regs[12] = &pattern[3];
  modbus.p_holding_regs[13] = &pattern[4];
  modbus.p_holding_regs[14] = &pattern[5];
  modbus.p_holding_regs[15] = &pattern[6];

  modbus.p_input_regs[0] = &MODEL_NO;
  modbus.p_input_regs[1] = &VERSION;
  modbus.p_input_regs[2] = &SUB_VERSION;
  for (int i = 3; i < 16; i++) {
    modbus.p_input_regs[i] = &na_adr;
  }
  for (int i = 16; i < 32; i++) {
    modbus.p_input_regs[i] = modbus.p_holding_regs[i-16];
  }

  //Cyclic Function0登録
  //Duty-intervalモード
  modbus.cycfunc0.rx_len = 3;
  modbus.cycfunc0.rx_adr[0] = &mode;
  modbus.cycfunc0.rx_adr[1] = &duty;
  modbus.cycfunc0.rx_adr[2] = &interval;
  modbus.cycfunc0.tx_len = 3;
  modbus.cycfunc0.tx_adr[0] = &mode;
  modbus.cycfunc0.tx_adr[1] = &duty;
  modbus.cycfunc0.tx_adr[2] = &interval;

  //Cyclic Function1登録
  //パターンモード
  modbus.cycfunc1.rx_len = 8;
  modbus.cycfunc1.rx_adr[0] = &pattern[0];
  modbus.cycfunc1.rx_adr[1] = &pattern[1];
  modbus.cycfunc1.rx_adr[2] = &pattern[2];
  modbus.cycfunc1.rx_adr[3] = &pattern[3];
  modbus.cycfunc1.rx_adr[4] = &pattern[4];
  modbus.cycfunc1.rx_adr[5] = &pattern[5];
  modbus.cycfunc1.rx_adr[6] = &pattern[6];
  modbus.cycfunc1.rx_adr[7] = &interval;
  modbus.cycfunc1.tx_len = 8;
  modbus.cycfunc1.tx_adr[0] = &pattern[0];
  modbus.cycfunc1.tx_adr[1] = &pattern[1];
  modbus.cycfunc1.tx_adr[2] = &pattern[2];
  modbus.cycfunc1.tx_adr[3] = &pattern[3];
  modbus.cycfunc1.tx_adr[4] = &pattern[4];
  modbus.cycfunc1.tx_adr[5] = &pattern[5];
  modbus.cycfunc1.tx_adr[6] = &pattern[6];
  modbus.cycfunc1.tx_adr[7] = &interval;

  ledcSetup(0, 12800, 10);
  ledcAttachPin(LED_PIN, 0);
  //Serial2.println("Start...");
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();
  static unsigned long previousMillis = 0;
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    int16_t led_duty;
    if (mode == 0) {
      if (led_on == 0) {
        led_on = 1;
        led_duty = 0;
      } else {
        led_on = 0;
        led_duty = duty;
      }
    }else if(mode == 1){
      led_duty = pattern[mode_cnt];
      mode_cnt++;
      if(mode_cnt>6) mode_cnt=0;
    }

    if(led_duty>duty_max) led_duty=duty_max;
    else if(led_duty<duty_min) led_duty=duty_min;
    ledcWrite(0, led_duty);
  }

  if (Serial.available()) {
    delay(10);
    uint8_t rbuf[50];
    int len = 0;
    while (Serial.available()) {
      rbuf[len] = Serial.read();
      len++;
    }
    //----デバック用----
    /*
    Serial2.print("Rx ");
    for (int i = 0; i < len; i++) {
      Serial2.print(rbuf[i], HEX);
      Serial2.print(' ');
    }
    Serial2.print("\r\n");
    */

    //modbus_taskでデータ処理
    uint8_t sbuf[50];
    int sl = modbus.modbus_task(rbuf, len, sbuf);
    Serial.write(sbuf, sl);

    //----デバック用----
    /*
    Serial2.print("Tx ");
    for (int i = 0; i < sl; i++) {
      Serial2.print(sbuf[i], HEX);
      Serial2.print(' ');
    }
    Serial2.print("\r\n");
    //------------------

    Serial2.print("coils ");
    for (int i = 0; i < 8; i++) {
      Serial2.print(*modbus.p_coils[i], HEX);
      Serial2.print(' ');
    }
    Serial2.println(' ');
    Serial2.print("regs ");
    for (int i = 0; i < 32; i++) {
      Serial2.print(*modbus.p_holding_regs[i], DEC);
      Serial2.print(' ');
    }
    Serial2.print("\r\n");
    */
  }
}
