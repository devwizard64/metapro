#ifndef _ABI_H_
#define _ABI_H_

#define A_SPNOOP        0x00
#define A_ADPCM         0x01
#define A_CLEARBUFF     0x02
#define A_ENVMIXER      0x03
#define A_LOADBUFF      0x04
#define A_RESAMPLE      0x05
#define A_SAVEBUFF      0x06
#define A_SEGMENT       0x07
#define A_SETBUFF       0x08
#define A_SETVOL        0x09
#define A_DMEMMOVE      0x0A
#define A_LOADADPCM     0x0B
#define A_MIXER         0x0C
#define A_INTERLEAVE    0x0D
#define A_POLEF         0x0E
#define A_SETLOOP       0x0F

#define A_INIT                  0x01
#define A_CONTINUE              0x00
#define A_LOOP                  0x02
#define A_OUT                   0x02
#define A_LEFT                  0x02
#define A_RIGHT                 0x00
#define A_VOL                   0x04
#define A_RATE                  0x00
#define A_AUX                   0x08
#define A_NOAUX                 0x00
#define A_MAIN                  0x00
#define A_MIX                   0x10

#endif /* _ABI_H_ */
