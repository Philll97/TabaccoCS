#include "task.h"


Task::Task(std::string task_name, uint16_t stack_size, uint8_t priority, BaseType_t core_id)
{
	str_task_name = task_name;
	ui16_stack_size = stack_size;
	ui8_priority  = priority;
	h_handle    = nullptr;
	v_task_data = nullptr;
	i_core_id	= core_id;
}

Task::~Task() {
} // ~Task

void Task::perform_command()
{
	this->log("Perform command");
	delay(1000);
}

void Task::run_task(void* pTaskInstance) {    
	Task* cur_task = (Task*) pTaskInstance;
	cur_task->perform_command();
	cur_task->stop();
} 

void Task::start() {
	if (h_handle != nullptr) {
		this->log("Task is already running!");
        return;
	}
	::xTaskCreatePinnedToCore(&run_task, str_task_name.c_str(), ui16_stack_size, this, ui8_priority, &h_handle, i_core_id);
} // start

void Task::stop()
{
    this->log("delete");
    if (h_handle == nullptr) return;
	::vTaskDelete(h_handle);
	h_handle = nullptr;
    this->log("Task deleted");
}

xTaskHandle Task::get_handle()
{
	return this->h_handle;
}

void Task::log(std::string msg)
{
	Serial.println(msg.c_str());
}