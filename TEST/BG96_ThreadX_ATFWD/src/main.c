/*
    ThreadX BG96 - MAIN
        Created on: 01.01.2019
        Author: Georgi Angelov
        http://www.wizio.eu/
        https://github.com/Wiz-IO/platform-quectel
 */

#include <_ansi.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include <qapi_types.h>
#include <qapi.h>
#include <qapi_status.h>
#include <qapi_uart.h>
#include <qapi_timer.h>
#include <qapi_device_info.h>
#include <qurt_timetick.h>
#include <quectel_utils.h>
#include <quectel_uart_apis.h>

void *_sbrk(intptr_t increment) { return (void *)-1; }

void abort(void)
{
    while (1) // dont block app
        qapi_Timer_Sleep(100, QAPI_TIMER_UNIT_SEC, 1);
}

TX_BYTE_POOL *heap;
static char heap_buffer[HEAP]; // build_flags = -D HEAP=xxx
void heap_init(void)
{
    if (txm_module_object_allocate(&heap, sizeof(TX_BYTE_POOL)))
        abort();
    if (tx_byte_pool_create(heap, "heap_byte_pool", heap_buffer, HEAP))
        abort();
}

static char *rx_buff = NULL;
static char *tx_buff = NULL;
static QT_UART_CONF_PARA uart_conf;

int quectel_dbg_uart_init(qapi_UART_Port_Id_e port_id)
{
    if (tx_byte_allocate(heap, (VOID *)&rx_buff, 4 * 1024, TX_NO_WAIT))
        abort();
    if (tx_byte_allocate(heap, (VOID *)&tx_buff, 4 * 1024, TX_NO_WAIT))
        abort();
    uart_conf.hdlr = NULL;
    uart_conf.port_id = port_id;
    uart_conf.tx_buff = tx_buff;
    uart_conf.tx_len = sizeof(tx_buff);
    uart_conf.rx_buff = rx_buff;
    uart_conf.rx_len = sizeof(rx_buff);
    uart_conf.baudrate = 115200;
    uart_init(&uart_conf); // open uart
    return TX_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

#include "qapi_fs_types.h"
#include "qapi_atfwd.h"
#include "quectel_utils.h"
#include "quectel_uart_apis.h"
typedef unsigned char boolean;
#define QUEC_AT_RESULT_ERROR_V01 0 /**<  Result ERROR.                                    \ \ \ \ \
                                         This is to be set in case of ERROR or CME ERROR. \ \ \ \ \
                                         The response buffer contains the entire details. */
#define QUEC_AT_RESULT_OK_V01 1 /**<  Result OK. This is to be set if the final response \    \    \    \    \
                                                     must send an OK result code to the terminal.       \ \ \ \ \
                                                     The response buffer must not contain an OK.  */
#define QUEC_AT_MASK_EMPTY_V01 0   /**<  Denotes a feed back mechanism for AT reg and de-reg */
#define QUEC_AT_MASK_NA_V01 1     /**<  Denotes presence of Name field  */
#define QUEC_AT_MASK_EQ_V01 2     /**<  Denotes presence of equal (=) operator  */
#define QUEC_AT_MASK_QU_V01 4     /**<  Denotes presence of question mark (?)  */
#define QUEC_AT_MASK_AR_V01 8     /**<  Denotes presence of trailing argument operator */
static int qexample_val = 0;

static int qt_atoi(char *str)
{
    int res = 0, i;
    for (i = 0; str[i] != '\0' && str[i] != '.'; ++i)
    {
        res = res * 10 + (str[i] - '0');
    }
    return res;
}

static void atfwd_cmd_handler_cb(boolean is_reg, char *atcmd_name,
                                 uint8 *at_fwd_params, uint8 mask,
                                 uint32 at_handle)
{
    char buff[32] = {0};
    int tmp_val = 0;
    qapi_Status_t ret = QAPI_ERROR;

    qt_uart_dbg(uart_conf.hdlr, "atfwd_cmd_handler_cb is called, atcmd_name:[%s] mask:[%d]", atcmd_name, mask);
    qt_uart_dbg(uart_conf.hdlr, "atfwd_cmd_handler_cb is called, is_reg:[%d]", is_reg);

    if (is_reg) //Registration Successful,is_reg return 1
    {
        if (mask == QUEC_AT_MASK_EMPTY_V01)
        {
            qt_uart_dbg(uart_conf.hdlr, "Atcmd %s is registered", atcmd_name);
            return;
        }
        if (!strncasecmp(atcmd_name, "+QEXAMPLE", strlen(atcmd_name)))
        {
            //Execute Mode
            if ((QUEC_AT_MASK_NA_V01) == mask) //AT+QEXAMPLE
            {
                ret = qapi_atfwd_send_resp(atcmd_name, "", QUEC_AT_RESULT_OK_V01);
            }
            //Read Mode
            else if ((QUEC_AT_MASK_NA_V01 | QUEC_AT_MASK_QU_V01) == mask) //AT+QEXAMPLE?
            {
                snprintf(buff, sizeof(buff), "+QEXAMPLE: %d", qexample_val);
                ret = qapi_atfwd_send_resp(atcmd_name, buff, QUEC_AT_RESULT_OK_V01);
            }
            //Test Mode
            else if ((QUEC_AT_MASK_NA_V01 | QUEC_AT_MASK_EQ_V01 | QUEC_AT_MASK_QU_V01) == mask) //AT+QEXAMPLE=?
            {
                snprintf(buff, sizeof(buff), "+QEXAMPLE: (0-2)");
                ret = qapi_atfwd_send_resp(atcmd_name, buff, QUEC_AT_RESULT_OK_V01);
            }
            //Write Mode
            else if ((QUEC_AT_MASK_NA_V01 | QUEC_AT_MASK_EQ_V01 | QUEC_AT_MASK_AR_V01) == mask) //AT+QEXAMPLE=<value>
            {
                tmp_val = qt_atoi((char *)at_fwd_params);
                if (tmp_val >= 0 && tmp_val <= 2)
                {
                    qexample_val = tmp_val;
                    ret = qapi_atfwd_send_resp(atcmd_name, "", QUEC_AT_RESULT_OK_V01);
                }
                else
                {
                    ret = qapi_atfwd_send_resp(atcmd_name, "", QUEC_AT_RESULT_ERROR_V01);
                }
            }
        }
        else
        {
            ret = qapi_atfwd_send_resp(atcmd_name, "", QUEC_AT_RESULT_ERROR_V01);
        }

        qt_uart_dbg(uart_conf.hdlr, "[%s] send resp, ret = %d", atcmd_name, ret);
    }
}

TX_BYTE_POOL *byte_pool_at;
UCHAR free_memory_at[10 * 1024];

__attribute__((section(".library"))) int TXM_MODULE_THREAD_ENTRY(void)
{
    heap_init();
    if (quectel_dbg_uart_init(QAPI_UART_PORT_002_E))
        abort();
    qt_uart_dbg(uart_conf.hdlr, "\nThreadX BG96 Hello World");

    /* wait 5sec for device startup */
    qapi_Timer_Sleep(5, QAPI_TIMER_UNIT_SEC, true);
    if (txm_module_object_allocate(&byte_pool_at, sizeof(TX_BYTE_POOL)))
        abort();
    if (tx_byte_pool_create(byte_pool_at, "byte_pool_at", free_memory_at, sizeof(free_memory_at)))
        abort();
    qt_uart_dbg(uart_conf.hdlr, "AT tx_byte_pool_create DONE");
    if (qapi_atfwd_Pass_Pool_Ptr(atfwd_cmd_handler_cb, byte_pool_at) != QAPI_OK)
    {
        qt_uart_dbg(uart_conf.hdlr, "Unable to alloc User space memory fail state  %x", 0);
    }
    else
    {
        qt_uart_dbg(uart_conf.hdlr, "qapi_atfwd_Pass_Pool_Ptr DONE");
    }
    int retval = qapi_atfwd_reg("+QEXAMPLE", atfwd_cmd_handler_cb);
    if (retval != QAPI_OK)
    {
        qt_uart_dbg(uart_conf.hdlr, "qapi_atfwd_reg  fail\n");
    }
    else
    {
        qt_uart_dbg(uart_conf.hdlr, "qapi_atfwd_reg  DONE");
    }

    while (1)
    {
        qt_uart_dbg(uart_conf.hdlr, "LOOP");
        tx_thread_sleep(1000);
    }
}
