#include "MyModbus.h"
#define DEV_ID 0x30

//UART Serial2(A4, A5);
MyModbus modbus(DEV_ID);

int16_t ro_value[16];
int16_t rw_value[16];
uint8_t dio=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Serial2.begin(115200);
  delay(2000);

  ro_value[0] = 48;   //モデルナンバー
  ro_value[1] = 0002; //バージョン
  ro_value[2] = 0000; //サブバージョン

  //各レジスタ初期化
  //コイル---rwレジスタと同じ
  modbus.p_coils[0] = &dio;//&rw_value[0];
  modbus.p_coils[1] = &dio;//&rw_value[1];

  for(int i=0; i<16; i++){
    modbus.p_holding_regs[i] = &rw_value[i];
    modbus.p_input_regs[i] = &ro_value[i];
  }
  for(int i=16; i<32; i++){
    modbus.p_input_regs[i] = &rw_value[i-16];
  }

  //Cyclic Function0登録
  modbus.cycfunc0.rx_len = 1;
  modbus.cycfunc0.rx_adr[0] = &rw_value[0];
  modbus.cycfunc0.tx_len = 1;
  modbus.cycfunc0.tx_adr[0] = &rw_value[0];

  //Cyclic Function1登録
  modbus.cycfunc1.rx_len = 2;
  modbus.cycfunc1.rx_adr[0] = &rw_value[0];
  modbus.cycfunc1.rx_adr[1] = &rw_value[1];
  modbus.cycfunc1.tx_len = 2;
  modbus.cycfunc1.tx_adr[0] = &rw_value[0];
  modbus.cycfunc1.tx_adr[1] = &rw_value[1];

  //Cyclic Function2登録
  modbus.cycfunc2.rx_len = 3;
  modbus.cycfunc2.rx_adr[0] = &rw_value[0];
  modbus.cycfunc2.rx_adr[1] = &rw_value[1];
  modbus.cycfunc2.rx_adr[2] = &rw_value[2];
  modbus.cycfunc2.tx_len = 3;
  modbus.cycfunc2.tx_adr[0] = &rw_value[0];
  modbus.cycfunc2.tx_adr[1] = &rw_value[1];
  modbus.cycfunc2.tx_adr[2] = &rw_value[2];

  //Cyclic Function3登録
  modbus.cycfunc3.rx_len = 1;
  modbus.cycfunc3.rx_adr[0] = &rw_value[0];
  modbus.cycfunc3.tx_len = 2;
  modbus.cycfunc3.tx_adr[0] = &rw_value[0];
  modbus.cycfunc3.tx_adr[1] = &rw_value[1];

  //Cyclic Function4登録
  modbus.cycfunc4.rx_len = 8;
  modbus.cycfunc4.rx_adr[0] = &rw_value[0];
  modbus.cycfunc4.rx_adr[1] = &rw_value[1];
  modbus.cycfunc4.rx_adr[2] = &rw_value[2];
  modbus.cycfunc4.rx_adr[3] = &rw_value[3];
  modbus.cycfunc4.rx_adr[4] = &rw_value[4];
  modbus.cycfunc4.rx_adr[5] = &rw_value[5];
  modbus.cycfunc4.rx_adr[6] = &rw_value[6];
  modbus.cycfunc4.rx_adr[7] = &ro_value[7];
  modbus.cycfunc4.tx_len = 8;
  modbus.cycfunc4.tx_adr[0] = &rw_value[0];
  modbus.cycfunc4.tx_adr[1] = &rw_value[1];
  modbus.cycfunc4.tx_adr[2] = &rw_value[2];
  modbus.cycfunc4.tx_adr[3] = &rw_value[3];
  modbus.cycfunc4.tx_adr[4] = &rw_value[4];
  modbus.cycfunc4.tx_adr[5] = &rw_value[5];
  modbus.cycfunc4.tx_adr[6] = &rw_value[6];
  modbus.cycfunc4.tx_adr[7] = &rw_value[7];

  //Serial2.println("Start...");
}

void loop() {
  // put your main code here, to run repeatedly:
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
    uint8_t sbuf[100];
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
  ro_value[7]++;
}
