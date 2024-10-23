/********************************************************************
 * File: logic_analyzer_task.c
 * Description: Logic analyzer task implementation.
 * Version: 1.0
 * Date: 2023-11-01
 * Author: zhengxinyu13@qq.com
 * ---------- Revision History ----------
 * <version> <date> <author> <desc>
 * 
 ********************************************************************/

#include "logic_analyzer_task.h"

static uint32_t g_virtual_buffer_size = 0x40000000;  /* 1GB */
static uint32_t g_max_frequency       = MAX_FREQUENCY;
static uint32_t g_sample_number       = 0;
static uint32_t g_sample_delay        = 0;
static uint32_t g_sampling_rate       = 0;
static uint32_t g_trigger_mask        = 0;
static uint32_t g_trigger_value       = 0;
static uint32_t g_flags               = 0;
static uint8_t  g_probes              = PROBES;
static uint8_t  g_trigger_state       = 0;

volatile uint8_t get_stop_cmd         = 0;

static uint8_t  g_rx_data_buf[BUFFER_SIZE];
static uint32_t g_rx_cnt_buf[BUFFER_SIZE];
static int32_t  g_cur_pos        = 0;        /* current position in the buffer */
static int32_t  g_cur_sample_cnt = 0;        /* current sample count */

void send_uint32(uint32_t value)
{
    uint8_t buffer[4];
    buffer[0] = BYTE0(value);
    buffer[1] = BYTE1(value);
    buffer[2] = BYTE2(value);
    buffer[3] = BYTE3(value);
    HAL_UART_Transmit(&huart1, buffer, sizeof(buffer), HAL_MAX_DELAY);
}

void send_id(void)
{
    HAL_UART_Transmit(&huart1, ID, strlen(ID), HAL_MAX_DELAY);
}

void send_device_name(void)
{
    uint8_t metadata = METADATA_TOKEN_DEVICE_NAME;
    HAL_UART_Transmit(&huart1, &metadata, sizeof(metadata), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, DEVICE_NAME, strlen(DEVICE_NAME), HAL_MAX_DELAY);
}

void send_memory_size(void)
{
    uint8_t metadata = METADATA_TOKEN_SAMPLE_MEMORY_BYTES;
    HAL_UART_Transmit(&huart1, &metadata, sizeof(metadata), HAL_MAX_DELAY);
    send_uint32(g_virtual_buffer_size);
    metadata = METADATA_TOKEN_DYNAMIC_MEMORY_BYTES;
    HAL_UART_Transmit(&huart1, &metadata, sizeof(metadata), HAL_MAX_DELAY);
    send_uint32(0);
}

void send_max_frequency(void)
{
    uint8_t metadata = METADATA_TOKEN_MAX_SAMPLE_RATE_HZ;
    HAL_UART_Transmit(&huart1, &metadata, sizeof(metadata), HAL_MAX_DELAY);
    send_uint32(g_max_frequency);
}

void send_probes(void)
{
    uint8_t metadata = METADATA_TOKEN_NUM_PROBES_SHORT;
    HAL_UART_Transmit(&huart1, &metadata, sizeof(metadata), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, &g_probes, sizeof(g_probes), HAL_MAX_DELAY);
}

void send_protocol_version(void)
{
    uint8_t metadata = METADATA_TOKEN_PROTOCOL_VERSION_SHORT;
    HAL_UART_Transmit(&huart1, &metadata, sizeof(metadata), HAL_MAX_DELAY);
    metadata = METADATA_TOKEN_FPGA_VERSION;
    HAL_UART_Transmit(&huart1, &metadata, sizeof(metadata), HAL_MAX_DELAY);
}

void send_end(void)
{
    uint8_t metadata = METADATA_TOKEN_END;
    HAL_UART_Transmit(&huart1, &metadata, sizeof(metadata), HAL_MAX_DELAY);
}

void collection_data(void)
{
    uint8_t data;
    uint8_t pre_data;
    uint32_t convreted_sample_count = g_sample_number * (MAX_FREQUENCY / g_sampling_rate);
    volatile uint16_t *data_reg = (volatile uint16_t *)0x40010C08; /* GPIOB_IDR */
    // volatile uint32_t *pa15_reg = (volatile uint32_t *)0x40010810; /* GPIOA_BSRR */

    get_stop_cmd = 0;
    g_cur_pos = 0;
    g_cur_sample_cnt = 0;

    Disable_TickIRQ(); /* Disable SysTick IRQ */

    memset(g_rx_cnt_buf, 0, BUFFER_SIZE);

    if (g_trigger_state && g_trigger_mask) {
        while (1) {
            data = (*data_reg) >> 8;
            if (data & g_trigger_mask & g_trigger_value)
                break;
            
            if (~data & g_trigger_mask & ~g_trigger_value)
                break;
            
            if (get_stop_cmd)
                return;
        }
    }

    data = (*data_reg) >> 8;
    g_rx_data_buf[0] = data;
    g_rx_cnt_buf[0] = 1;
    g_cur_sample_cnt = 1;
    pre_data = data;

    while(1) {
        data = (*data_reg) >> 8;

        g_cur_pos += (data != pre_data) ? 1 : 0;
        g_rx_data_buf[g_cur_pos] = data;
        g_rx_cnt_buf[g_cur_pos]++;
        g_cur_sample_cnt++;
        pre_data = data;

        if (get_stop_cmd)
            break;
        
        if (g_cur_sample_cnt >= convreted_sample_count)
            break;

        if (g_cur_pos >= BUFFER_SIZE - 1)
            break;

        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
        __asm volatile( "nop" );
    }
    Enable_TickIRQ(); /* Enable SysTick IRQ */
}

void report_data(void)
{
    uint32_t i = g_cur_pos;
    uint32_t j;
    uint32_t count = 0;
    uint32_t rate = MAX_FREQUENCY / g_sampling_rate;

    for (; i >= 0; i--) {
        for (j = 0; j < g_rx_cnt_buf[i]; j++) {
            count++;
            if (count == rate) {
                HAL_UART_Transmit(&huart1, &g_rx_data_buf[i], 1, HAL_MAX_DELAY);
                count = 0;
            }
        }
    }
}

void run(void)
{
    collection_data();
    report_data();
}

void set_sample_number(uint32_t sample_number)
{
    g_sample_number = (sample_number > g_virtual_buffer_size) ? g_virtual_buffer_size : sample_number;
}

void set_sample_delay(uint32_t sample_delay)
{
    g_sample_delay = sample_delay;
}

void set_sampling_divider(uint32_t divider)
{
    int sampling_rate = CLOCK_FREQUENCY / (divider + 1);

    if (sampling_rate > MAX_FREQUENCY)
        g_sampling_rate = MAX_FREQUENCY;
    else
        g_sampling_rate = sampling_rate;

}

void set_trigger_mask(uint32_t trigger_mask)
{
    g_trigger_mask = trigger_mask;
}

void set_trigger_value(uint32_t trigger_value)
{
    g_trigger_value = trigger_value;
}

void set_trigger_state(uint8_t trigger_state)
{
    g_trigger_state = trigger_state;
}

void set_flags(uint8_t flags)
{
    g_flags = flags;
}

void logic_analyzer_task(void *pvParameters)
{
    uint8_t cmd_buffer[5];
    uint8_t cmd_index = 0;
    uint8_t c;
    uint32_t temp;

    while (1) {
        if (UART1GetCharTimeout(&c, HAL_MAX_DELAY) == HAL_OK) {
            cmd_buffer[cmd_index] = c;
            switch (cmd_buffer[0]) {
            case CMD_RESET:
                break;
            case CMD_ID:
                send_id();
                break;
            case CMD_METADATA:
                send_device_name();
                send_end();
                send_memory_size();
                send_max_frequency();
                send_probes();
                send_protocol_version();
                send_end();
                break;
            case CMD_ARM_BASIC_TRIGGER:
                run();
                break;
            case CMD_XON:
                break;
            case CMD_XOFF:
                break;
            case CMD_CAPTURE_SIZE:
                cmd_index++;
                if (cmd_index < 5)
                    continue;
                
                temp = *((uint16_t *)(cmd_buffer + 1));
                set_sample_number(temp * 4);
                temp = *((uint16_t *)(cmd_buffer + 3));
                set_sample_delay(temp * 4);
                break;
            case CMD_SET_DIVIDER:
                cmd_index++;
                if (cmd_index < 5)
                    continue;

                temp = *((uint32_t *)(cmd_buffer + 1));
                set_sampling_divider(temp);
                break;
            case CMD_SET_BASIC_TRIGGER_MASK0:
                cmd_index++;
                if (cmd_index < 5)
                    continue;

                temp = *((uint32_t *)(cmd_buffer + 1));
                set_trigger_mask(temp);
                break;
            case CMD_SET_BASIC_TRIGGER_VALUE0:
                cmd_index++;
                if (cmd_index < 5)
                    continue;

                temp = *((uint32_t *)(cmd_buffer + 1));
                set_trigger_value(temp);
                break;
            case CMD_SET_BASIC_TRIGGER_CONFIG0:
                cmd_index++;
                if (cmd_index < 5)
                    continue;

                uint8_t serial = (*((uint8_t*)(cmd_buffer + 4)) & 0x04) > 0 ? 1 : 0;
				uint8_t state = (*((uint8_t*)(cmd_buffer + 4)) & 0x08) > 0 ? 1 : 0; 

                if (serial)
                    set_trigger_state(0); // Not supported
                else
                    set_trigger_state(state);
                break;
            case CMD_SET_FLAGS:
                cmd_index++;
                if (cmd_index < 5)
                    continue;

                temp = *((uint32_t *)(cmd_buffer + 1));
                set_flags(temp);
                break;
            case CMD_CAPTURE_DELAYCOUNT:
                cmd_index++;
                if (cmd_index < 5)
                    continue;

                temp = *((uint32_t *)(cmd_buffer + 1));
                set_sample_delay(temp * 4);
                break;
            case CMD_CAPTURE_READCOUNT:
                cmd_index++;
                if (cmd_index < 5)
                    continue;

                temp = *((uint32_t *)(cmd_buffer + 1));
                set_sample_number(temp * 4);
                break;
            default:
                break;
            }
            cmd_index = 0;
            memset(cmd_buffer, 0, 5);
        }
    }
}