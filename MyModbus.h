// Modbus通信対応のスレーブデバイスのサンプル
//実装ファンクションコードは下記の通り
//Read Holding Register（3）
//Read Input Register（4）
//Force Single Coil（5）
//Preset Single Register（6）
//Preset Multiple Registers（16）
//例外レスポンス（Exception Response）
//シリアルデータの送受信は別途実装する必要あり
//レジスタ空間は下記の通り
//Holding Register uint16_t × 32
//Input Register uint16_t × 32
//Coil uint16_t × 16 コイル数は8*8 = 64
//データは下記要領でアドレスを登録する。
//MyModbus modbus(DEV_ID);
//uint16_t value;
//modbus.p_holding_regs[0] = &value;
//
//
//
//

# ifndef _MYMODBUS_H_
# define _MYMODBUS_H_

#include "Arduino.h"

// 構造体
struct sCycFunc {
    int16_t tx_len = 0;
    int16_t rx_len = 0;
    int16_t *tx_adr[8];
    int16_t *rx_adr[8];
};
     
// クラス宣言
class MyModbus
    {
    public:
        MyModbus(uint8_t dev_adr);
        // void set_holding_regs(int adr, uint16_t *val_adr);
        // void set_input_regs(int adr, uint16_t *val_adr);
        int modbus_task(uint8_t *rbuf, int rl, uint8_t *s_buf);
        int16_t *p_input_regs[32];
        uint8_t *p_coils[2];
        int16_t *p_holding_regs[16];
        const int REGS_MAX = 32;
        const int COILS_MAX = 8;
        sCycFunc cycfunc0;
        sCycFunc cycfunc1;
        sCycFunc cycfunc2;
        sCycFunc cycfunc3;
        sCycFunc cycfunc4;
    private:
        uint8_t id;
    };
     
# endif //_MYMODBUS_H_