#pragma once
#include <string>
#include <stdarg.h>

#include <fstream>
#include <ctime>
#include <format>

class Logger
{
public:

	inline static std::ofstream s_LogFile;
	
	static void Init()
	{
		s_LogFile.open("Log.txt");
	}
	
	template<typename... Args>
	static void Info(const std::string& format, Args&&... args)
	{
		std::time_t timestamp = std::time(nullptr);
		std::tm* local_time = std::localtime(&timestamp);
		char buffer[80];
		std::strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", local_time);
		std::string msg = std::vformat(
			format,
			std::make_format_args(args...)
		);
		s_LogFile << "[" << buffer << "][INFO] " << msg;
		s_LogFile.flush();
	}
	
	template<typename... Args>
	static void Warning(const std::string& format, Args&&... args)
	{
		std::time_t timestamp = std::time(nullptr);
		std::tm* local_time = std::localtime(&timestamp);
		char buffer[80];
		std::strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", local_time);
		std::string msg = std::vformat(
			format,
			std::make_format_args(args...)
		);
		s_LogFile << "[" << buffer << "][WARNING] " << msg;
		s_LogFile.flush();
	}
	
	template<typename... Args>
	static void Error(const std::string& format, Args&&... args)
	{
		std::time_t timestamp = std::time(nullptr);
		std::tm* local_time = std::localtime(&timestamp);
		char buffer[80];
		std::strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", local_time);
		std::string msg = std::vformat(
			format,
			std::make_format_args(args...)
		);
		s_LogFile << "[" << buffer << "][ERROR] " << msg;
		s_LogFile.flush();
	}

};

