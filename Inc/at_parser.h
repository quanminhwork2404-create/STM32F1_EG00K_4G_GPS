#ifndef AT_PARSER_H
#define AT_PARSER_H

#include <stdint.h>

typedef enum
{
    AT_OK = 0,
    AT_ERROR,
    AT_TIMEOUT
} AT_Status_t;

void AT_Init(void);

AT_Status_t AT_SendCommand(
    const char *cmd,
    const char *expected_resp,
    uint32_t timeout_ms
);

AT_Status_t AT_SendCommandResponse(
    const char *cmd,
    char *response,
    uint16_t max_len,
    uint32_t timeout_ms
);

int AT_WaitForResponse(
    const char *expected_resp,
    uint32_t timeout_ms,
    char *out_buffer,
    uint16_t max_len
);

void AT_SendData(const char *data);

#endif
