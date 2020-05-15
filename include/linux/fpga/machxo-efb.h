
#define CTRL_OFFSET		0

#define FPGA_READ	(1 << 15)
#define FPGA_WRITE	(0 << 15)

#define AD_CNT(x)	((((x) - 1) & 0x7) << 12)
#define AD_ADDR(x)	((x) & 0x1FFF)



int efb_spi_read(u16 reg);
int efb_spi_write(u16 reg, u8 val);

