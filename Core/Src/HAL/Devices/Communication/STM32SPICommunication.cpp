
#include "STM32SPICommunication.h"

#include <map>

namespace HAL 
{
namespace Devices 
{
namespace Communication 
{

std::map<SPI_HandleTypeDef*, STM32SPICommunication*> group_of_SPIs;

STM32SPICommunication::STM32SPICommunication( const SPIInterface::SPIConfiguration &spi_config )
                        : SPIInterface( spi_config ),
                          spi_handle_( std::make_unique<SPI_HandleTypeDef>() )
{
    spi_handle_->Instance = BaseSPIToHalSPINummber(  spi_config.spi_number );

    if( spi_config.spi_mode == SPIMode::Master )
        spi_handle_->Init.Mode = SPI_MODE_MASTER;
    else
        spi_handle_->Init.Mode = SPI_MODE_SLAVE;
    
    spi_handle_->Init.Direction = SPI_DIRECTION_2LINES;

    if( spi_config.spi_data_size == SPIDataSize::SPI_8Bits ) 
        spi_handle_->Init.DataSize = SPI_DATASIZE_8BIT;
    else
        spi_handle_->Init.DataSize = SPI_DATASIZE_16BIT;

    FromBaseSPIModeToHalPolarity( spi_config.spi_timming_mode );
    PrescalerCalculation( spi_config.spi_baud_selector );

    spi_handle_->Init.NSS = SPI_NSS_SOFT;
  
    spi_handle_->Init.FirstBit = SPI_FIRSTBIT_MSB;
    spi_handle_->Init.TIMode = SPI_TIMODE_DISABLE;
    spi_handle_->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi_handle_->Init.CRCPolynomial = 10;

    group_of_SPIs.insert( {spi_handle_.get(), this} );

    ConfigureCSPin();

    HAL_GPIO_WritePin( GPIOA, GPIO_PIN_4, GPIO_PIN_SET );

    HAL_SPI_Init(spi_handle_.get());
}

STM32SPICommunication::~STM32SPICommunication()
{

}

void STM32SPICommunication::ConfigureCSPin()
{   
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

bool STM32SPICommunication::SetCSPin( uint8_t pin_value )
{
    GPIO_PinState state = (pin_value == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    HAL_GPIO_WritePin( GPIOA, GPIO_PIN_4, state );
    return true;
}

bool STM32SPICommunication::WriteDataIT( const uint8_t *data, uint16_t data_size, std::function<void(void)> callback_write_finish )
{   
    callback_write_finish_ = callback_write_finish;
    return (HAL_SPI_Transmit_IT( spi_handle_.get(), data, data_size ) == HAL_OK);
}

bool STM32SPICommunication::WriteData( const uint8_t *data, uint16_t data_size ) 
{   
    return (HAL_SPI_Transmit( spi_handle_.get(), data, data_size, kTxTimeoutMs ) == HAL_OK);
}

bool STM32SPICommunication::ReadData( uint8_t *read_buffer, uint16_t data_size )
{
    return ( HAL_SPI_Receive( spi_handle_.get(), read_buffer, data_size, kRxTimeoutMs ) == HAL_OK );
}

bool STM32SPICommunication::ReadDataIT( uint8_t *read_buffer, uint16_t data_size, std::function<void(void)> callback_read_finish )
{
    callback_read_finish_ = callback_read_finish;
    return ( HAL_SPI_Receive_IT( spi_handle_.get(), read_buffer , data_size ) == HAL_OK );
}

bool STM32SPICommunication::WriteReadData( const uint8_t *data_write, uint8_t *read_data, uint16_t data_size ) 
{
    return (HAL_SPI_TransmitReceive( spi_handle_.get(), data_write, read_data, data_size, kRxTimeoutMs ) == HAL_OK);
}

SPI_TypeDef *STM32SPICommunication::BaseSPIToHalSPINummber( SPINumber spi_number )
{
	switch (spi_number) 
    {
		case SPINumber::SPI_1:
			return SPI1;
		case SPINumber::SPI_2:
			return SPI2;
		case SPINumber::SPI_3:
			return SPI3;
		default:
			return SPI3;
	}
}

void STM32SPICommunication::FromBaseSPIModeToHalPolarity( SPITimmingMode timming_mode )
{
    switch ( timming_mode )
    {
    case SPITimmingMode::CPOL0_CPHA0:
        spi_handle_->Init.CLKPolarity = SPI_POLARITY_LOW;
        spi_handle_->Init.CLKPhase = SPI_PHASE_1EDGE;
        break;
    case SPITimmingMode::CPOL0_CPHA1:
        spi_handle_->Init.CLKPolarity = SPI_POLARITY_LOW;
        spi_handle_->Init.CLKPhase = SPI_PHASE_2EDGE;
        break;
    case SPITimmingMode::CPOL1_CPHA0:
        spi_handle_->Init.CLKPolarity = SPI_POLARITY_HIGH;
        spi_handle_->Init.CLKPhase = SPI_PHASE_1EDGE;
        break;
    case SPITimmingMode::CPOL1_CPHA1:
        spi_handle_->Init.CLKPolarity = SPI_POLARITY_HIGH;
        spi_handle_->Init.CLKPhase = SPI_PHASE_2EDGE;
        break;
    default:
        break;
    }
}

void STM32SPICommunication::PrescalerCalculation( SPIBaudRatePrescaler prescaler )
{
    switch (prescaler)
    {
        case SPIBaudRatePrescaler::BaudRatePrescaler_2:
            spi_handle_->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
            break;
        case SPIBaudRatePrescaler::BaudratePrescaler_4:
            spi_handle_->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
            break;
        case SPIBaudRatePrescaler::BaudratePrescaler_8:
            spi_handle_->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
            break;
        case SPIBaudRatePrescaler::BaudratePrescaler_16:
            spi_handle_->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
            break;
        case SPIBaudRatePrescaler::BaudratePrescaler_32:
            spi_handle_->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
            break;
        case SPIBaudRatePrescaler::BaudratePrescaler_64:
            spi_handle_->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
            break;
        case SPIBaudRatePrescaler::BaudratePrescaler_128:
            spi_handle_->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
        default:
            break;
    }
}
extern "C" {

void SPI1_IRQHandler(void)
{
    STM32SPICommunication *current_spi = nullptr;
    for(auto &spi : group_of_SPIs) {
        if(spi.first->Instance == SPI1) {
            current_spi = spi.second;
        }
    }

    if(current_spi == nullptr) {
        return;
    }

    HAL_SPI_IRQHandler( current_spi->spi_handle_.get() );
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    auto it = group_of_SPIs.find(hspi);
    if(it == group_of_SPIs.end()) {
        return;
    }

    STM32SPICommunication *current_spi = it->second;
    if(current_spi->callback_write_finish_) 
    {
        current_spi->callback_write_finish_();
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    auto it = group_of_SPIs.find(hspi);
    if(it == group_of_SPIs.end()) {
        return;
    }

    STM32SPICommunication *current_spi = it->second;
    if(current_spi->callback_read_finish_) 
    {
        current_spi->callback_read_finish_();
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    // auto it = group_of_SPIs.find(hspi);
    // if(it == group_of_SPIs.end()) {
    //     return;
    // }

    // STM32SPICommunication *current_spi = it->second;
    // Handle error - could log or trigger error callback
    // For now, just clear the error flag
    __HAL_SPI_CLEAR_OVRFLAG(hspi);
}

}

}
}
}
