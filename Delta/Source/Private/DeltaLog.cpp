#include "stdafx.h"
#include "DeltaLog.h"
#include <iostream>

#include <fstream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <mutex>

// Fully portable: no platform-specific console APIs

static std::mutex logMutex;
std::ofstream DeltaLog::logFile;
std::string DeltaLog::logFileName;
std::string DeltaLog::logFolder = "Logs";
int DeltaLog::logIndent = 0;

void DeltaLog::increaseIndent()
{
	logIndent++;
}
void DeltaLog::decreaseIndent()
{
	logIndent--;
	logIndent = logIndent < 0 ? 0 : logIndent;
}

void DeltaLog::init(const std::string& logFilename)
{
	logFileName = logFilename;

	namespace fs = std::filesystem;

	fs::create_directories(logFolder);
	std::string fullLogPath = (fs::path(logFolder) / logFilename).string();

	if (fs::exists(fullLogPath))
	{
		renameOldLogFile(fullLogPath);
	}

	// Open new log file
	logFile.open(fullLogPath, std::ios::out | std::ios::trunc);
	if (!logFile.is_open())
	{
		throw std::runtime_error("Failed to open log file: " + fullLogPath);
	}
}

void DeltaLog::renameOldLogFile(const std::string& oldFilePath)
{
	namespace fs = std::filesystem;

	auto ftime = fs::last_write_time(oldFilePath);

	auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
		ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now()
	);

	std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &cftime);
#else
    localtime_r(&cftime, &localTime);
#endif

	char buffer[64];
	std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &localTime);

	// Build new file name with timestamp inside the logFolder
	fs::path oldPath(oldFilePath);
	std::string newFileName = "log_" + std::string(buffer) + oldPath.extension().string();

	fs::path newPath = fs::path(logFolder) / newFileName;

	// Rename (move) old log file into folder with timestamped name
	fs::rename(oldFilePath, newPath);
}

void DeltaLog::print( const std::string& Message, ELog Type )
{
	DeltaLog::print(Message.c_str(), Type);
}

void DeltaLog::print(const char* const Message, ELog Type)
{
	std::string indent(logIndent * indentSize, ' ');

	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
	// Convert to localtime
    std::tm local_tm{};
#if defined(_WIN32)
    localtime_s(&local_tm, &now_time_t);
#else
    localtime_r(&now_time_t, &local_tm);
#endif

	std::lock_guard<std::mutex> lock(logMutex);

    char debugMessage[512];
    const char* typeStr = nullptr;

    switch (Type)
    {
    case ELog::Success:
        typeStr = "Success";
        break;
    case ELog::Log:
        typeStr = "Log";
        break;
    case ELog::Warning:
        typeStr = "Warning";
        break;
    case ELog::Error:
        typeStr = "Error";
        break;
    case ELog::Fatal:
        typeStr = "Fatal";
        break;
    }

    // ANSI colors for non-Windows
    const char* color = "\033[0m";
    switch (Type)
    {
    case ELog::Success: color = "\033[32m"; break; // green
    case ELog::Log:     color = "\033[0m";  break; // reset
    case ELog::Warning: color = "\033[33m"; break; // yellow
    case ELog::Error:   color = "\033[31m"; break; // red
    case ELog::Fatal:   color = "\033[41;37m"; break; // red background, white text
    }
    std::cout << color << indent << Message << "\033[0m" << std::endl;

	if (logFile.is_open())
	{
		std::stringstream prefix_ss;
		prefix_ss << std::put_time(&local_tm, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << now_ms.count();
		prefix_ss << " " << typeStr << ":";
		const int prefixWidth = 22 + (logIndent * indentSize);
		std::string prefix = prefix_ss.str();
		logFile << std::left << std::setw(prefixWidth) << prefix << " " << Message << std::endl;

		logFile.flush();
	}
}
