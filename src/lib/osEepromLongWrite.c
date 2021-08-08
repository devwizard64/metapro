void lib_osEepromLongWrite(void)
{
    u64 *data = malloc(0x200);
    __eeprom_read(data, 0x200);
    __BYTESWAP(&data[a1], __tlb(a2), a3);
    __eeprom_write(data, 0x200);
    v0 = 0;
}
