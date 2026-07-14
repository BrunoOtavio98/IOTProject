#ifndef STM32_SPI_COMMUNICATION_H
#define STM32_SPI_COMMUNICATION_H

#include "Interfaces/SPIInterface.h"

#include <memory>

#include "stm32f4xx_hal.h"

namespace HAL 
{
namespace Devices 
{
namespace Communication 
{

class STM32SPICommunication : public HAL::Devices::Communication::Interfaces::SPIInterface
{

public:
    static const int kChunkSize = 512;
    static constexpr int kTxTimeoutMs = 200;
    static constexpr int kRxTimeoutMs = 400;

    std::unique_ptr<SPI_HandleTypeDef> spi_handle_;
    std::function<void(void)> callback_read_finish_;
    std::function<void(void)> callback_write_finish_;

    STM32SPICommunication( const SPIInterface::SPIConfiguration &spi_config );
    ~STM32SPICommunication();

    bool WriteData( const uint8_t *data, uint16_t data_size ) override;
    bool WriteDataIT( const uint8_t *data, uint16_t data_size, std::function<void(void)> callback_write_finish ) override;
    bool ReadData( uint8_t *read_buffer, uint16_t data_size ) override;
	bool ReadDataIT( uint8_t *read_buffer, uint16_t data_size, std::function<void(void)> callback_read_finish ) override;
    bool WriteReadData( const uint8_t *data_write, uint8_t *data_read, uint16_t data_size ) override;
    bool SetCSPin( uint8_t pin_value ) override;

private:
    uint8_t buffer_read_size[kChunkSize];

    SPI_TypeDef *BaseSPIToHalSPINummber( SPINumber spi_number );
    void FromBaseSPIModeToHalPolarity( SPITimmingMode timming_mode );
    void PrescalerCalculation( SPIBaudRatePrescaler prescaler );
    void ConfigureCSPin();
};



}
}
}

#endif // STM32_SPI_COMMUNICATION_H
