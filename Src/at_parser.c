
#include "at_parser.h"
#include "uart.h"
#include "debug.h"
#include <string.h>
#include <stdint.h>

#define AT_RESPONSE_SIZE 512

void AT_Init(void)
{
    UART_Init();
    SysTick_Init();
}

/* Kiem tra mot dong phan hoi co dung bang token hay khong.
 * Chap nhan CRLF, LF hoac CR o cuoi dong.
 */
static int AT_HasLine(const char *response, const char *token)
{
    const char *p = response;
    size_t token_len = strlen(token);

    while (*p)
    {
        const char *start;
        const char *end;
        size_t len;

        while (*p == '\r' || *p == '\n')
            p++;

        if (*p == '\0')
            break;

        start = p;

        while (*p && *p != '\r' && *p != '\n')
            p++;

        end = p;
        len = (size_t)(end - start);

        if (len == token_len && strncmp(start, token, token_len) == 0)
            return 1;

        /* Xu ly dong cuoi chua co CR/LF: chi nhan neu token
         * da duoc ket thuc boi ky tu ket thuc chuoi.
         */
        if (*p == '\0' && len == token_len &&
            strncmp(start, token, token_len) == 0)
            return 1;
    }

    return 0;
}

static int AT_HasError(const char *response)
{
    if (AT_HasLine(response, "ERROR"))
        return 1;

    if (strstr(response, "+CME ERROR:") != 0)
        return 1;

    if (strstr(response, "+CMS ERROR:") != 0)
        return 1;

    return 0;
}

AT_Status_t AT_SendCommandResponse(
    const char *cmd,
    char *response,
    uint16_t max_len,
    uint32_t timeout_ms)
{
    uint16_t idx = 0;
    uint32_t start;
    char c;

    if (cmd == 0 || response == 0 || max_len < 2)
        return AT_ERROR;

    response[0] = '\0';

    /* Bo du lieu cu dang cho truoc khi gui lenh moi. */
    UART_Flush();

    DEBUG_Printf("\r\n[AT TX] %s\r\n", cmd);

    UART_SendString(cmd);
    UART_SendString("\r\n");

    start = Millis();

    while ((uint32_t)(Millis() - start) < timeout_ms)
    {
        while (UART_ReadChar(&c))
        {
            if (idx < max_len - 1)
            {
                response[idx++] = c;
                response[idx] = '\0';
            }

            /* Ket thuc thanh cong khi co dong OK rieng. */
            if (AT_HasLine(response, "OK"))
            {
                DEBUG_Printf("[AT RX - OK]\r\n%s\r\n", response);
                return AT_OK;
            }

            if (AT_HasError(response))
            {
                DEBUG_Printf("[AT RX - ERROR]\r\n%s\r\n", response);
                return AT_ERROR;
            }
        }
    }

    DEBUG_Printf("[AT TIMEOUT] cmd=%s, bytes=%u\r\n",
                 cmd, (unsigned int)idx);

    if (idx > 0)
        DEBUG_Printf("[AT PARTIAL RX]\r\n%s\r\n", response);
    else
        DEBUG_Printf("[AT PARTIAL RX] (no data)\r\n");

    return AT_TIMEOUT;
}

AT_Status_t AT_SendCommand(
    const char *cmd,
    const char *expected_resp,
    uint32_t timeout_ms)
{
    char response[AT_RESPONSE_SIZE];

    AT_Status_t status = AT_SendCommandResponse(
        cmd, response, sizeof(response), timeout_ms);

    if (status != AT_OK)
        return status;

    if (expected_resp != 0 &&
        strstr(response, expected_resp) == 0)
        return AT_ERROR;

    return AT_OK;
}

int AT_WaitForResponse(
    const char *expected_resp,
    uint32_t timeout_ms,
    char *out_buffer,
    uint16_t max_len)
{
    uint16_t idx = 0;
    uint32_t start = Millis();
    char c;

    if (out_buffer == 0 || max_len < 2)
        return 0;

    out_buffer[0] = '\0';

    while ((uint32_t)(Millis() - start) < timeout_ms)
    {
        while (UART_ReadChar(&c))
        {
            if (idx < max_len - 1)
            {
                out_buffer[idx++] = c;
                out_buffer[idx] = '\0';
            }

            if (expected_resp != 0 &&
                strstr(out_buffer, expected_resp) != 0)
            {
                DEBUG_Printf("[AT WAIT RX]\r\n%s\r\n", out_buffer);
                return 1;
            }
        }
    }

    DEBUG_Printf("[AT WAIT TIMEOUT]\r\n%s\r\n",
                 idx ? out_buffer : "(no data)");
    return 0;
}

void AT_SendData(const char *data)
{
    UART_SendString(data);
}
