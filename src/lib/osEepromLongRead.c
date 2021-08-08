void lib_osEepromLongRead(void)
{
    u64 *data = malloc(0x200);
    __eeprom_read(data, 0x200);
    __BYTESWAP(__tlb(a2), &data[a1], a3);
    v0 = 0;
}
