#ifndef SLASK_SPY_INCLUDE_COMMON_LOGGER
#define SLASK_SPY_INCLUDE_COMMON_LOGGER

#include <cstdarg>
#include <iostream>
#include <vector>

class LoggerImpl {
public:
	virtual void Info(const char *format) const = 0;
	virtual void Warn(const char *format) const = 0;
	virtual void Error(const char *format) const = 0;
};

class Logger {
public:
	static void CreateContext(LoggerImpl *logger_implementation)
	{
		Get().impl_ = logger_implementation;
	}

	static Logger &Get()
	{
		static Logger inst{};
		return inst;
	}

	static void Info(const char *format, ...)
	{
		Logger const &logger{Logger::Get()};
		if (logger.impl_ == nullptr) {
			std::cerr << "Logger used without initializing context"
				  << std::endl;
			return;
		}

		va_list args{};
		va_start(args, format);
		const int32_t size =
			vsnprintf(nullptr, 0, format, args) + sizeof('\0');
		va_end(args);

		char *buffer = new char[size];
		va_start(args, format);
		vsnprintf(buffer, size, format, args);
		logger.impl_->Info(buffer);
		va_end(args);
	}

	static void Warn(const char *format, ...)
	{
		Logger const &logger{Logger::Get()};
		if (logger.impl_ == nullptr) {
			std::cerr << "Logger used without initializing context"
				  << std::endl;
			return;
		}

		va_list args{};
		va_start(args, format);
		const int32_t size =
			vsnprintf(nullptr, 0, format, args) + sizeof('\0');
		va_end(args);

		char *buffer = new char[size];
		va_start(args, format);
		vsnprintf(buffer, size, format, args);
		logger.impl_->Warn(buffer);
		va_end(args);
	}

	static void Error(const char *format, ...)
	{
		Logger const &logger{Logger::Get()};
		if (logger.impl_ == nullptr) {
			std::cerr << "Logger used without initializing context"
				  << std::endl;
			return;
		}

		va_list args{};
		va_start(args, format);
		const int32_t size =
			vsnprintf(nullptr, 0, format, args) + sizeof('\0');
		va_end(args);

		char *buffer = new char[size];
		va_start(args, format);
		vsnprintf(buffer, size, format, args);
		logger.impl_->Error(buffer);
		va_end(args);
	}

	~Logger()
	{
		if (impl_ != nullptr) {
			delete impl_;
		}
	}

private:
	Logger() : impl_{nullptr} {}

	LoggerImpl const *impl_;
};

#endif // SLASK_SPY_INCLUDE_COMMON_LOGGER
