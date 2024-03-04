#ifndef SLASK_SPY_OBS_SRC_LOGGER
#define SLASK_SPY_OBS_SRC_LOGGER

#include <obs-module.h>
#include <plugin-support.h>

#include <cstdarg>

#include "logger.h"

class OBSLogger : public LoggerImpl {
	
	void Info(const char* format) const override {
		obs_log(LOG_INFO, format);
	}

	void Warn(const char *format) const override
	{
		obs_log(LOG_WARNING, format);
	}

	void Error(const char *format) const override
	{
		obs_log(LOG_ERROR, format);
	}
};

#endif  // SLASK_SPY_OBS_SRC_LOGGER
