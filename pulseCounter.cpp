
// ********   P U L S E   C O U N T E R   H E A D E R  *************
// ********   BEGIN

#include "pulseCounter.h"

// This is first run in setup()  -->  pcnt config convers the unit and channel of Pcnt  -->  GPIO of pulsePin and pulseInGate
void initPcnt(byte pin)
{

	Serial.println("init pulse counter... ");

	// setup pcnt_config_t
	pcnt_config_t pcnt_config =
	{
		pin,												     	    // = GPIO5 Pulse input gpio_num, if you want to use gpio16, pulse_gpio_num = 16, a negative value will be ignored
		PCNT_PIN_NOT_USED,										//  = -1 Control signal input gpio_num, a negative value will be ignored
		PCNT_MODE_KEEP,												// = 0 PCNT low control mode
		PCNT_MODE_KEEP,												// PCNT high control mode
		PCNT_COUNT_INC,												// PCNT positive edge count mode
		PCNT_COUNT_DIS,												// PCNT negative edge count mode
		PCNT_H_LIM_VAL,												// Maximum counter value
		PCNT_L_LIM_VAL,												// Minimum counter value
		PCNT_TEST_UNIT,												// PCNT unit number
		PCNT_CHANNEL_0,												// the PCNT channel
	};

	//Setting up of particular channel is then done by calling a function pcnt_unit_config() with above pcnt_config_t as the input parameter.
	if (pcnt_unit_config(&pcnt_config) == ESP_OK)					//init unit
		Serial.println("Config Unit_0 = ESP_OK");

	// filter value
	//pcnt_set_filter_value(PCNT_TEST_UNIT, 1);
	//pcnt_filter_enable(PCNT_TEST_UNIT);

	// * @brief Disable PCNT interrupt for PCNT unit  * @param pcnt_unit PCNT unit number  * @return  *     - ESP_OK Success *     - ESP_ERR_INVALID_ARG Parameter error  -->  esp_err_t pcnt_intr_disable(pcnt_unit_t pcnt_unit);
	pcnt_intr_disable(PCNT_TEST_UNIT);

	// * @brief Disable PCNT event of PCNT unit  * @param unit PCNT unit number  * @param evt_type Watch point event type. All enabled events share the same interrupt (one interrupt per pulse counter unit). * @return *     - ESP_OK Success *     - ESP_ERR_INVALID_ARG Parameter error  -->  esp_err_t pcnt_event_disable(pcnt_unit_t unit, pcnt_evt_type_t evt_type);
	pcnt_event_disable(PCNT_TEST_UNIT, PCNT_EVT_L_LIM);
	pcnt_event_disable(PCNT_TEST_UNIT, PCNT_EVT_H_LIM);
	pcnt_event_disable(PCNT_TEST_UNIT, PCNT_EVT_THRES_0);
	pcnt_event_disable(PCNT_TEST_UNIT, PCNT_EVT_THRES_1);
	pcnt_event_disable(PCNT_TEST_UNIT, PCNT_EVT_ZERO);

	pcnt_counter_pause(PCNT_TEST_UNIT);								// Pause counter
	pcnt_counter_clear(PCNT_TEST_UNIT);								// Reset counter value

																	//  * @brief Enable PCNT interrupt for PCNT unit * @note Each Pulse counter unit has five watch point events that share the same interrupt. *        Configure events with pcnt_event_enable() and pcnt_event_disable() * @param pcnt_unit PCNT unit number
	pcnt_intr_enable(PCNT_TEST_UNIT);

	pcnt_counter_resume(PCNT_TEST_UNIT);							// Resume counting

}
