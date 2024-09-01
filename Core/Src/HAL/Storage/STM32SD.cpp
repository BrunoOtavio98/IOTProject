
#include "Storage/STM32SD.h"

namespace HAL {
namespace Storage {

STM32SD::STM32SD()
{

}

STM32SD::~STM32SD()
{
}

bool STM32SD::InitStorage() {

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_4B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 0;
  if (HAL_SD_Init(&hsd) != HAL_OK)
  {
    return false;
  }
  if (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B) != HAL_OK)
  {
    return false;
  }

  HAL_SD_CardInfoTypeDef info;
  HAL_SD_GetCardInfo(&hsd, &info);
  return true;
}

// void SDIO_IRQHandler(void)
// {
//   /* USER CODE BEGIN SDIO_IRQn 0 */

//   /* USER CODE END SDIO_IRQn 0 */
//   HAL_SD_IRQHandler(&hsd);
//   /* USER CODE BEGIN SDIO_IRQn 1 */

//   /* USER CODE END SDIO_IRQn 1 */
// }

}
}
