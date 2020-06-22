
static uint32_t spi_transfer(spi_context *ctx, uint8_t *wr_buf, uint8_t *rd_buf, unsigned int size)
{
    if (spi_in_use(ctx))
        return 0;
    if (size > SPI_MAX_PACKET_LENGTH)
        size = SPI_MAX_PACKET_LENGTH; // 1024
    memcpy(spi_tx_buffer, wr_buf, size);
    SPI_TX_ADDR_REG = (int)spi_tx_buffer;
    SPI_RX_ADDR_REG = (int)spi_rx_buffer;
    SPI_COMM.DMA_EN_TX = SPI_MODE_DMA;
    SPI_COMM.DMA_EN_RX = SPI_MODE_DMA;
    SPI_CONF1.PACKET_LENGTH = size - 1; // max 1024
    SPI_CONF1.PACKET_LOOP_CNT = 0;

    SPI_COMM.ACT = 1;
    while (SPI_STATUS1_REG & 1) // wait finish
    {
    }
    SPI_STATUS1_REG = 0; // clear finish

    memcpy(rd_buf, spi_rx_buffer, size); // return rx
    return size;
}

int SPI_Transfer(spi_context *ctx, uint8_t *wr_buf, uint8_t *rd_buf, uint16_t size)
{
    if (0 == size || NULL == wr_buf || NULL == rd_buf)
        return 0;    
    int res, cnt = 0;
    while (size)
    {
        if (0 == (res = spi_transfer(ctx, wr_buf, rd_buf, size)))
            break;
        size -= res;
        cnt += res;
        wr_buf += res;
        rd_buf += res;
    }
    return cnt;
}
