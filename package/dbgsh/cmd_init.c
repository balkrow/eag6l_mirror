void cmd_parse_init()
{
	cmd_common_init();
	cmd_i2c_init();
	cmd_memory_init();
#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-25 */
	cmd_setup_init();
#endif
}
