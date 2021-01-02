void lib_osEepromLongWrite(void)
{
    u64 *data = malloc(0x200);
    __eeprom_read(data, 0x200);
    __BYTESWAP(&data[a1.i[IX]], __tlb(a2.i[IX]), a3.i[IX]);
    __eeprom_write(data, 0x200);
    v0.ll = (s32)0;
}
