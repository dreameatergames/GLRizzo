sh-elf-objcopy -R .stack -O binary nxglquake.elf makaqu.bin

$KOS_BASE/utils/scramble/scramble makaqu.bin 1st_read.bin