void lib_osEepromLongRead(void)
{
    u64 *data = malloc(0x200);
    __eeprom_read(data, 0x200);
    __BYTESWAP(__tlb(a2.i[IX]), &data[a1.i[IX]], a3.i[IX]);
    v0.ll = (s32)0;
}
