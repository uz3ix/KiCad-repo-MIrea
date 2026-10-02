#ifndef CHECKSUM_H
#define CHECKSUM_H
/* 16-разрядная аддитивная контрольная сумма, по модулю 65536.
 * unsigned short имеет 16 бит на 8051 (Keil C51 и SDCC).
 */
static unsigned short checksum_add(unsigned short sum, unsigned char value)
{
    return (unsigned short)(sum + (unsigned short)value);
}
#endif
