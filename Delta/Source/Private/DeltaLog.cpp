#include "stdafx.h"
#include "DeltaLog.h"
#include <iostream>
#include <windows.h>

#include <fstream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <mutex>

static std::mutex logMutex;
std::ofstream DeltaLog::logFile;
std::string DeltaLog::logFileName;
std::string DeltaLog::logFolder = "..\\Logs";
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

	std::tm localTime;
	localtime_s(&localTime, &cftime);

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
	std::tm local_tm;
	localtime_s(&local_tm, &now_time_t);

	std::lock_guard<std::mutex> lock(logMutex);

	static HANDLE hstdout = GetStdHandle( STD_OUTPUT_HANDLE );
	
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo( hstdout, &csbi );

	char debugMessage[512];
	const char* typeStr = nullptr;

	switch (Type)
	{
	case ELog::Success:
		SetConsoleTextAttribute( hstdout, FOREGROUND_GREEN );
		snprintf(debugMessage, sizeof(debugMessage), "Log: %s%s\n", indent.c_str(),Message);
		typeStr = "Success";
		break;
	case ELog::Log:
		SetConsoleTextAttribute( hstdout, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
		snprintf(debugMessage, sizeof(debugMessage), "Log: %s%s\n", indent.c_str(),Message);
		typeStr = "Log";
		break;
	case ELog::Warning:
		SetConsoleTextAttribute( hstdout, FOREGROUND_GREEN | FOREGROUND_RED );
		snprintf(debugMessage, sizeof(debugMessage), "Warning: %s%s\n", indent.c_str(),Message);
		std::cout <<"Warning: ";
		typeStr = "Warning";
		break;
	case ELog::Error:
		SetConsoleTextAttribute( hstdout, FOREGROUND_RED );
		snprintf(debugMessage, sizeof(debugMessage), "Error: %s%s\n", indent.c_str(),Message);
		std::cout <<"Error: ";
		typeStr = "Error";
		break;
	case ELog::Fatal:
		SetConsoleTextAttribute( hstdout, BACKGROUND_RED | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE );
		snprintf(debugMessage, sizeof(debugMessage), "Fatal: %s%s\n", indent.c_str(),Message);
		std::cout <<"Fatal: ";
		typeStr = "Fatal";
		break;
	}

	std::cout << indent << Message << std::endl;
	OutputDebugString(debugMessage);
	
	SetConsoleTextAttribute( hstdout, csbi.wAttributes );

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