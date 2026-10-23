
#include "stm32f103xb.h"
#include "debug.h"
#include "uart.h"
#include "at_parser.h"

#include <stdint.h>
#include <string.h>

int main(void)
{
    char response[512];
    AT_Status_t status;

    DEBUG_Init();
    AT_Init();

    DEBUG_Printf("\r\n=== EG800K GNSS FIX TEST ===\r\n");

    Delay_ms(1000);

    /* Kiem tra GNSS dang bat */
    AT_SendCommandResponse(
        "AT+QGPS?", response, sizeof(response), 2000
    );

    while (1)
    {
        DEBUG_Printf("\r\n--- Query GNSS location ---\r\n");

        status = AT_SendCommandResponse(
            "AT+QGPSLOC=2",
            response,
            sizeof(response),
            3000
        );

        if (status == AT_OK)
        {
            if (strstr(response, "+QGPSLOC:") != 0)
            {
                DEBUG_Printf("[GNSS] Co du lieu toa do.\r\n");
            }
            else
            {
                DEBUG_Printf(
                    "[GNSS] Lenh thanh cong nhung khong co QGPSLOC.\r\n"
                );
            }
        }
        else if (status == AT_TIMEOUT)
        {
            DEBUG_Printf("[GNSS] Timeout khi doc toa do.\r\n");
        }
        else
        {
            DEBUG_Printf(
                "[GNSS] Chua lay duoc toa do, xem CME ERROR/ERROR o log.\r\n"
            );
        }

        Delay_ms(5000);
    }
}
