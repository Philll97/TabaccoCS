#ifndef DEFINES_HEADER
#define DEFINES_HEADER

#include <Arduino.h>
#include <vector>
#include <Arduino_JSON.h>

// General Settings
#define MACHINE_TYPE    "TABACCO"
#define MACHINE_ID      1

#define MAX_TUBES       39
#define I2C_START_ADDR  0x20
#define I2C_CONFIG_ADDR 0x11

// UART Settings
#define UART_MSG_SIZE   6
#define UART_BAUDRATE   9600
#define UART_CONFIG     SERIAL_8N1
#define UART_RX_PIN     9
#define UART_TX_PIN     10
#define UART_RX_TIMEOUT 3
#define UART_MAX_RETRY  5

// MQTT Settings
#define MQTT_BROKER     "192.168.1.129"
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

#define JSON_KEY_ACKN               "ackn"
#define JSON_VAL_REQ                "request"
#define JSON_VAL_ACKN               "acknowledged"

#define JSON_KEY_ERROR              "error"
#define JSON_VAL_NO_ERROR           "no error"
#define JSON_VAL_TUBE_NR_WRONG      "wrong tube number"
#define JSON_VAL_COMMAND_ERR        "invalid command"
#define JSON_VAL_UART_ERR           "uart communication failed"
#define JSON_VAL_MQTT_PARSE_FAILED  "mqtt message could not be parsed"
#define JSON_VAL_TUBE_EMPTY         "tube empty"

#define JSON_KEY_FAILED_COUNT       "failed_count"
#define JSON_KEY_FAILED_TUBES       "failed_tubes"
#define JSON_KEY_RELEASE_SUCCESS    "success"

// Ethernet Settings
#define ETH_MACADDR     0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED

#define ETH_FIXED_IP    0
#define ETH_IPADDR      192,168,0,160
#define ETH_IPMASK    255,255,255,0
#define ETH_DNS       192,168,0,1
#define ETH_GW        192,168,0,1

// Communication States
enum class communication_states
{
    idle,
    waiting_for_msg,
    new_msg,
    error
};
// Machine Tasks
enum class machine_tasks
{
    idle,
    health_check,       //check communication
    check_if_emtpy,     //check if tube has product
    release_content,    //eject a product
    set_i2c_address,     //set i2c address while configuring
    error
};

enum class check_if_empty_sub_tasks
{
    none,
    set_output_A,
    get_digital_state,
    reset_output_A,
};

enum class release_content_sub_tasks //for release content
{
    none,
    put_tube_in_standby,
    check_if_ready,
    eject_product,
    check_if_finished,
    reset_pusher_position,
    check_if_empty,
    reset_output_A
};

enum class release_content_sub_sub_tasks
{
    none,
    set_output_A,
    set_output_B,
    reset_output_A,
    reset_output_B
};

extern machine_tasks current_task;
extern release_content_sub_tasks release_content_sub_task; 
extern release_content_sub_sub_tasks release_content_sub_sub_task; 

// Machine Commands
enum class machine_command_types
{
    health_check,       //check communication
    check_if_emtpy,     //check if tube has product
    release_content,    //eject a product
    set_i2c_address     //set i2c address while configuring
};

struct machine_command
{ 
    String command_type;
    uint8_t tube_nr;
    String ackn;
};

/* struct machine_reply_health_check
{
    String command_type;
    bool success;
    JSONVar tube_nrs; //if failed send tube nr which failed
    String ackn;
};

struct machine_reply_release_content
{
    String command_type;
    uint8_t tube_nr;
    bool success;
    String error;
    String ackn;
};

struct machine_reply_empty
{
    String command_type;
    uint8_t tube_nr;
    bool empty;
    String error;
    String ackn;
};

struct machine_reply_set_i2c_address
{
    String command_type;
    uint8_t tube_nr;
    bool success;
    String error;
    String ackn;
}; */


// Peripherie Commands
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