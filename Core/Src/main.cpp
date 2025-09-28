
#include "main.h"
#include "HAL/Boards/BoardInterface.h"
#include "HAL/Boards/STM32Board.h"

using HAL::Boards::BoardInterface;
using HAL::Boards::STM32Board;

BoardInterface *board = nullptr;

int main(void)
{
  board = new STM32Board();
  board->InitPeripherals(BoardInterface::AvailableModemInterfaces::SIM_800L);

  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
