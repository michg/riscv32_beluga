int a1 = 1 / (L'\x1234' - 0x1234);            /* div by 0 */
int a2 = 1 / (L'\x3412' - 0x1234);
int a3 = 1 / (L'\x1234' - 0x34120000);
int a4 = 1 / (L'\x12340000' - 0x12340000);    /* div by 0 */
int a5 = 1 / (L'\x00003412' - 0x12340000);
int a6 = 1 / (L'\x12345678' - 0x12345678);    /* div by 0 */
int a7 = 1 / (L'\x78563412' - 0x12345678);
