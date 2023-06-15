#ifndef TASK_HEADER
#define TASK_HEADER

#include <Arduino.h>
#include "defines.h"

class Task
{
    private:
        xTaskHandle h_handle;
        std::string str_task_name;
        uint16_t    ui16_stack_size;
        uint8_t     ui8_priority;
        BaseType_t  i_core_id;
        void*       v_task_data;  
        static void run_task(void* data);
        
    public:
        Task(std::string task_name, uint16_t stack_size = 10000, uint8_t priority = 10, BaseType_t core_id = 0);
        ~Task();

        void start();//(void* task_data = nullptr);
        void stop();
        xTaskHandle get_handle();

        virtual void perform_command();//(void* data) = 0;
        virtual void log(std::string msg);

};

#endif