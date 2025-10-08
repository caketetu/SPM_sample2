// Modbus通信対応のスレーブデバイスのサンプル
#include "MyModbus.h"
#include "FS.h"
#include "SPIFFS.h"

#define UNKOWN_FUNC 1
#define INVAILED_ADR 2
#define INVAILED_DATA 3

// buf		受信データ
// length	受信データ長(CRCを除く)
uint16_t calc_crc(uint8_t *buf, int length) {
  uint16_t crc = 0xFFFF;
  int i, j;
  uint8_t carrayFlag;
  for (i = 0; i < length; i++) {
    crc ^= buf[i];
    for (j = 0; j < 8; j++) {
      carrayFlag = crc & 1;
      crc = crc >> 1;
      if (carrayFlag) {
        crc ^= 0xA001;
      }
    }
  }
  return crc;
}

uint16_t convert_uint16(uint8_t h_data, uint8_t l_data) {
  uint16_t conv_data = (uint16_t)(h_data << 8) + l_data;
  return conv_data;
}

int16_t convert_int16(uint8_t h_data, uint8_t l_data) {
  int16_t conv_data = (int16_t)(h_data << 8) + l_data;
  return conv_data;
}

void make_exception_resp(uint8_t *buf, uint8_t id, uint8_t fc, uint8_t ec) {
  buf[0] = id;
  buf[1] = fc | 0x80;
  buf[2] = ec;
  uint16_t crc = calc_crc(buf, 3);
  buf[4] = crc;
  buf[5] = (uint8_t)(crc >> 8);
  return;
}

MyModbus::MyModbus(){}

int MyModbus::modbus_task(uint8_t *rbuf, int rl, uint8_t *sbuf) {
  if (rbuf[0] == id) {
    uint16_t crc_check = convert_uint16(rbuf[rl - 1], rbuf[rl - 2]);
    if (crc_check == calc_crc(rbuf, rl - 2)) {
      switch (rbuf[1]) {
          //保持ステータス読み出し
          //パラメータなどを読み出す
          // Read Holding Register（03）
        case 3:
          {
            uint16_t adr = convert_uint16(rbuf[2], rbuf[3]);
            uint16_t data_len = convert_uint16(rbuf[4], rbuf[5]);
            //レジスタ不正エラー
            if (adr + data_len > REGS_MAX) {
              make_exception_resp(sbuf, id, rbuf[1], INVAILED_ADR);
              return 5;
            }
            int sbuf_len = 0;
            sbuf[0] = id;
            sbuf[1] = rbuf[1];
            sbuf[2] = data_len * 2;
            sbuf_len = 3;
            for (int i = adr; i < adr + data_len; i++) {
              sbuf[sbuf_len++] = (uint8_t)(*p_holding_regs[i] >> 8);
              sbuf[sbuf_len++] = *p_holding_regs[i];
            }

            uint16_t crc = calc_crc(sbuf, sbuf_len);
            sbuf[sbuf_len++] = crc;
            sbuf[sbuf_len++] = (uint8_t)(crc >> 8);
            return sbuf_len;
          }
          break;

          //入力ステータス読み出し
          //システムステータスや変数などを読み出す
          // Read Input Register（04）
        case 4:
          {
            uint16_t adr = convert_uint16(rbuf[2], rbuf[3]);
            uint16_t data_len = convert_uint16(rbuf[4], rbuf[5]);
            //レジスタ不正エラー
            if (adr + data_len > REGS_MAX) {
              make_exception_resp(sbuf, id, rbuf[1], INVAILED_ADR);
              return 5;
            }
            int sbuf_len = 0;
            sbuf[0] = id;
            sbuf[1] = rbuf[1];
            sbuf[2] = data_len * 2;
            sbuf_len = 3;
            for (int i = adr; i < adr + data_len; i++) {
              sbuf[sbuf_len++] = (uint8_t)(*p_input_regs[i] >> 8);
              sbuf[sbuf_len++] = *p_input_regs[i];
            }

            uint16_t crc = calc_crc(sbuf, sbuf_len);
            sbuf[sbuf_len++] = crc;
            sbuf[sbuf_len++] = (uint8_t)(crc >> 8);
            return sbuf_len;
          }
          break;

          //単一コイルに書き込む
          //イネーブルやモードの変更、パラメータの保存フラグなど
          // Force Single Coil（05）
        case 5:
          {
            uint16_t adr = convert_uint16(rbuf[2], rbuf[3]);
            uint16_t value = convert_uint16(rbuf[4], rbuf[5]);
            int index = adr / 8;
            int coils_value = 0x01 << (adr % 8);

            //レジスタ不正エラー
            if (adr > COILS_MAX * 16) {
              make_exception_resp(sbuf, id, rbuf[1], INVAILED_ADR);
              return 5;
            }

            //レジスタ書き換え
            if (value == 0xFF00) {
              *p_coils[index] = *p_coils[index] | coils_value;
            } else if (value == 0x0000) {
              *p_coils[index] = *p_coils[index] & ~coils_value;
            } else {
              //データ不正エラー
              make_exception_resp(sbuf, id, rbuf[1], INVAILED_DATA);
              return 5;
            }
            sbuf[0] = id;
            sbuf[1] = rbuf[1];
            sbuf[2] = rbuf[2];
            sbuf[3] = rbuf[3];
            sbuf[4] = rbuf[4];
            sbuf[5] = rbuf[5];
            uint16_t crc = calc_crc(sbuf, 6);
            sbuf[6] = crc;
            sbuf[7] = (uint8_t)(crc >> 8);
            return 8;
          }
          break;

          //保持レジスタ書き込み
          //単一の保持レジスタの内容を書き換えます
          // Preset Single Register（06)
        case 6:
          {
            uint16_t adr = convert_uint16(rbuf[2], rbuf[3]);
            //レジスタ不正エラー
            if (adr > REGS_MAX) {
              make_exception_resp(sbuf, id, rbuf[1], INVAILED_ADR);
              return 5;
            }
            *p_holding_regs[adr] = convert_int16(rbuf[4], rbuf[5]);
            sbuf[0] = id;
            sbuf[1] = rbuf[1];
            sbuf[2] = rbuf[2];
            sbuf[3] = rbuf[3];
            sbuf[4] = rbuf[4];
            sbuf[5] = rbuf[5];
            uint16_t crc = calc_crc(sbuf, 6);
            sbuf[6] = crc;
            sbuf[7] = (uint8_t)(crc >> 8);
            return 8;
          }
          break;

          //保持レジスタ書き込み
          //複数の連続した保持レジスタ領域へ書き込みます。
          // Preset Multiple Registers（16，0x10）
        case 16:
          {
            uint16_t adr = convert_uint16(rbuf[2], rbuf[3]);
            uint16_t data_len = convert_uint16(rbuf[4], rbuf[5]);
            uint16_t n_byte = rbuf[6];
            //レジスタ不正エラー
            if (adr + data_len > REGS_MAX) {
              make_exception_resp(sbuf, id, rbuf[1], INVAILED_ADR);
              return 5;
            }

            for (int i = 7; i < n_byte + 7; i += 2) {
              *p_holding_regs[adr++] = convert_int16(rbuf[i], rbuf[i + 1]);
            }
            sbuf[0] = id;
            sbuf[1] = rbuf[1];
            sbuf[2] = rbuf[2];
            sbuf[3] = rbuf[3];
            sbuf[4] = rbuf[4];
            sbuf[5] = rbuf[5];
            uint16_t crc = calc_crc(sbuf, 6);
            sbuf[6] = crc;
            sbuf[7] = (uint8_t)(crc >> 8);
            return 8;
          }
          break;

        //Cyclic Function 0
        case 65:
          {
            sCycFunc cf = cycfunc0;
            int rc = 2;
            for (int i = 0; i < cf.tx_len; i++) {
              *cf.tx_adr[i] = convert_int16(rbuf[rc++], rbuf[rc++]);
            }

            rc = 0;
            sbuf[rc++] = id;
            sbuf[rc++] = rbuf[1];
            for (int i = 0; i < cf.rx_len; i++) {
              sbuf[rc++] = (uint8_t)(*cf.rx_adr[i] >> 8);
              sbuf[rc++] = *cf.rx_adr[i];
            }
            uint16_t crc = calc_crc(sbuf, rc);
            sbuf[rc++] = crc;
            sbuf[rc++] = (uint8_t)(crc >> 8);
            return rc;
          }

        //Cyclic Function 1
        case 66:
          {
            sCycFunc cf = cycfunc1;
            int rc = 2;
            for (int i = 0; i < cf.tx_len; i++) {
              *cf.tx_adr[i] = convert_int16(rbuf[rc++], rbuf[rc++]);
            }

            rc = 0;
            sbuf[rc++] = id;
            sbuf[rc++] = rbuf[1];
            for (int i = 0; i < cf.rx_len; i++) {
              sbuf[rc++] = (uint8_t)(*cf.rx_adr[i] >> 8);
              sbuf[rc++] = *cf.rx_adr[i];
            }
            uint16_t crc = calc_crc(sbuf, rc);
            sbuf[rc++] = crc;
            sbuf[rc++] = (uint8_t)(crc >> 8);
            return rc;
          }

        //Cyclic Function 2
        case 67:
          {
            sCycFunc cf = cycfunc2;
            int rc = 2;
            for (int i = 0; i < cf.tx_len; i++) {
              *cf.tx_adr[i] = convert_int16(rbuf[rc++], rbuf[rc++]);
            }

            rc = 0;
            sbuf[rc++] = id;
            sbuf[rc++] = rbuf[1];
            for (int i = 0; i < cf.rx_len; i++) {
              sbuf[rc++] = (uint8_t)(*cf.rx_adr[i] >> 8);
              sbuf[rc++] = *cf.rx_adr[i];
            }
            uint16_t crc = calc_crc(sbuf, rc);
            sbuf[rc++] = crc;
            sbuf[rc++] = (uint8_t)(crc >> 8);
            return rc;
          }

        //Cyclic Function 3
        case 68:
          {
            sCycFunc cf = cycfunc3;
            int rc = 2;
            for (int i = 0; i < cf.tx_len; i++) {
              *cf.tx_adr[i] = convert_int16(rbuf[rc++], rbuf[rc++]);
            }

            rc = 0;
            sbuf[rc++] = id;
            sbuf[rc++] = rbuf[1];
            for (int i = 0; i < cf.rx_len; i++) {
              sbuf[rc++] = (uint8_t)(*cf.rx_adr[i] >> 8);
              sbuf[rc++] = *cf.rx_adr[i];
            }
            uint16_t crc = calc_crc(sbuf, rc);
            sbuf[rc++] = crc;
            sbuf[rc++] = (uint8_t)(crc >> 8);
            return rc;
          }

        //Cyclic Function 4
        case 69:
          {
            sCycFunc cf = cycfunc4;
            int rc = 2;
            for (int i = 0; i < cf.tx_len; i++) {
              *cf.tx_adr[i] = convert_int16(rbuf[rc++], rbuf[rc++]);
            }

            rc = 0;
            sbuf[rc++] = id;
            sbuf[rc++] = rbuf[1];
            for (int i = 0; i < cf.rx_len; i++) {
              sbuf[rc++] = (uint8_t)(*cf.rx_adr[i] >> 8);
              sbuf[rc++] = *cf.rx_adr[i];
            }
            uint16_t crc = calc_crc(sbuf, rc);
            sbuf[rc++] = crc;
            sbuf[rc++] = (uint8_t)(crc >> 8);
            return rc;
          }

        default:
          make_exception_resp(sbuf, id, rbuf[1], UNKOWN_FUNC);
          return 5;
          break;
      }
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}
