#ifndef DEFINES_HEADER
#define DEFINES_HEADER

#include <Arduino.h>
#include <vector>
#include <Arduino_JSON.h>

// General Settings
#define MACHINE_TYPE    "TABACCO"
#define MACHINE_ID      1

#define TUBE_CNT        39
#define I2C_START_ADDR  0x20
#define I2C_CONFIG_ADDR 0x11
#define MOTOR_BLOCKED_MAX_CNT 5

// UART Settings
#define UART_MSG_SIZE   6
#define UART_BAUDRATE   9600
#define UART_CONFIG     SERIAL_8N1
#define UART_RX_PIN     9
#define UART_TX_PIN     10
#define UART_RX_TIMEOUT 1
#define UART_MAX_RETRY  2

// MQTT Settings
#define MQTT_BROKER     "192.168.0.154"
#define MQTT_PORT       1883
#define MQTT_SUB_TOPIC  "tabacco/input"
#define MQTT_PUB_TOPIC  "tabacco/output"

// MQTT Message Keys and Values
#define JSON_KEY_COMMAND            "command"
#define JSON_VAL_HEALTH_CHECK       "health_check"
#define JSON_VAL_CHECK_IF_EMPTY     "check_if_empty"
#define JSON_VAL_RELEASE_CONTENT    "release_content"
#define JSON_VAL_SET_I2C_ADDRESS    "set_i2c_address"

#define JSON_KEY_TUBE_NR            "tube_nr"
#define JSON_KEY_TUBE_NRS           "tube_nrs"

#define JSON_KEY_ACKN               "ackn"
#define JSON_VAL_REQ                "request"
#define JSON_VAL_ACKN               "acknowledged"

#define JSON_KEY_ERROR              "error"
#define JSON_VAL_NO_ERROR           "no_error"
#define JSON_VAL_TUBE_NR_WRONG      "wrong_tube_number"
#define JSON_VAL_TUBE_NOT_CONF      "not_configured"
#define JSON_VAL_COMMAND_ERR        "invalid_command"
#define JSON_VAL_UART_ERR           "uart_communication_failed"
#define JSON_VAL_MQTT_PARSE_FAILED  "mqtt_message_could_not_be_parsed"
#define JSON_VAL_TUBE_EMPTY         "tube_empty"
#define JSON_VAL_TUBE_EMPTIED       "tube_emptied"
#define JSON_VAL_MOTOR_BLOCKED      "motor_blocked"

#define JSON_KEY_FAILED_COUNT       "failed_count"
#define JSON_KEY_FAILED_TUBES       "failed_tubes"
#define JSON_KEY_RELEASE_SUCCESS    "success"

// Tasks
#define TASK_STATE_CREATED "task_created"
#define TASK_STATE_FINISHED "finished"

// Ethernet Settings
#define ETH_MACADDR     0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED

#define ETH_FIXED_IP    0
#define ETH_IPADDR      192,168,0,160
#define ETH_IPMASK    255,255,255,0
#define ETH_DNS       192,168,0,1
#define ETH_GW        192,168,0,1

// Communication
enum class communication_states
{
    idle,
    waiting_for_msg,
    new_msg,
    error
};

// Modul
enum class modul_command_types
{
    health_check,       //check communication
    check_if_emtpy,     //check if tube has product
    release_content,    //eject a product
    set_i2c_address     //set i2c address while configuring
};


// Peripherie
enum class peripherie_command_types
{
    check_communication     = 1,
    set_i2c_address         = 2,
    output_on               = 4,
    output_off              = 5,
    output_impulse          = 7,
    set_pwm_freq            = 8,
    set_pwm_duty_cycle      = 9,
    start_pwm               = 11,
    stop_pwm                = 15,
    read_digital_inputs     = 16,
    read_analog_input       = 32
};

enum class peripherie_states
{
    ready                   = 3,
    command_understood      = 5,
    message_error           = 9,
    command_error           = 17,
    not_configured          = 33
};

struct peripherie_command
{
    uint8_t address;                        //i2c address of peripherie board
    peripherie_command_types command_type;
    uint8_t data1 = 0;                           //address / port -> which output/input is used
    uint8_t data2 = 0;                           //dutycycle / frequency HIGH
    uint8_t data3 = 0;                           //frequency LOW
};

struct peripherie_reply
{
    uint8_t address;                        //i2c address of peripherie board
    peripherie_command_types command_type;
    peripherie_states state;                
    uint8_t data1;                           //GPIO0 State / ADC value HIGH
    uint8_t data2;                           //GPIO1 State / ADC value LOW
};


#endif