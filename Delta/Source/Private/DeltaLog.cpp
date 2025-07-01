#include "stdafx.h"
#include "DeltaLog.h"
#include <iostream>
#include <windows.h>

#include <fstream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <mutex>

static std::mutex LogMutex;
std::ofstream DeltaLog::LogFile;
std::string DeltaLog::LogFileName;
std::string DeltaLog::LogFolder = "..\\Logs";

void DeltaLog::Init(const std::string& logFilename)
{
	LogFileName = logFilename;

	namespace fs = std::filesystem;

	fs::create_directories(LogFolder);
	std::string fullLogPath = (fs::path(LogFolder) / logFilename).string();

	if (fs::exists(fullLogPath))
	{
		RenameOldLogFile(fullLogPath);
	}

	// Open new log file
	LogFile.open(fullLogPath, std::ios::out | std::ios::trunc);
	if (!LogFile.is_open())
	{
		throw std::runtime_error("Failed to open log file: " + fullLogPath);
	}
}

void DeltaLog::RenameOldLogFile(const std::string& oldFilePath)
{
	namespace fs = std::filesystem;

	auto ftime = fs::last_write_time(oldFilePath);

	auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
		ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now()
	);

	std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

	std::tm localTime;
#ifdef _WIN32
	localtime_s(&localTime, &cftime);
#else
	localtime_r(&cftime, &localTime);
#endif

	char buffer[64];
	std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &localTime);

	// Build new file name with timestamp inside the logFolder
	fs::path oldPath(oldFilePath);
	std::string newFileName = "log_" + std::string(buffer) + oldPath.extension().string();

	fs::path newPath = fs::path(LogFolder) / newFileName;

	// Rename (move) old log file into folder with timestamped name
	fs::rename(oldFilePath, newPath);
}

void DeltaLog::Print( const std::string& Message, ELog Type )
{
	DeltaLog::Print(Message.c_str(), Type);
}

void DeltaLog::Print(const char* const Message, ELog Type)
{

	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
	// Convert to localtime
	std::tm local_tm;
#ifdef _WIN32
	localtime_s(&local_tm, &now_time_t);
#else
	localtime_r(&now_time_t, &local_tm);
#endif

	std::lock_guard<std::mutex> lock(LogMutex);

	static HANDLE hstdout = GetStdHandle( STD_OUTPUT_HANDLE );
	
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo( hstdout, &csbi );

	char debugMessage[512];
	const char* typeStr = nullptr;

	switch (Type)
	{
	case ELog::Success:
		SetConsoleTextAttribute( hstdout, FOREGROUND_GREEN );
		snprintf(debugMessage, sizeof(debugMessage), "Log: %s\n", Message);
		typeStr = "Success";
		break;
	case ELog::Log:
		SetConsoleTextAttribute( hstdout, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
		snprintf(debugMessage, sizeof(debugMessage), "Log: %s\n", Message);
		typeStr = "Log";
		break;
	case ELog::Warning:
		SetConsoleTextAttribute( hstdout, FOREGROUND_GREEN | FOREGROUND_RED );
		snprintf(debugMessage, sizeof(debugMessage), "Warning: %s\n", Message);
		std::cout <<"Warning: ";
		typeStr = "Warning";
		break;
	case ELog::Error:
		SetConsoleTextAttribute( hstdout, FOREGROUND_RED );
		snprintf(debugMessage, sizeof(debugMessage), "Error: %s\n", Message);
		std::cout <<"Error: ";
		typeStr = "Error";
		break;
	case ELog::Fatal:
		SetConsoleTextAttribute( hstdout, FOREGROUND_RED );
		snprintf(debugMessage, sizeof(debugMessage), "Fatal: %s\n", Message);
		std::cout <<"Fatal: ";
		typeStr = "Fatal";
		break;
	}

	std::cout<< Message <<std::endl;
	OutputDebugString(debugMessage);
	
	SetConsoleTextAttribute( hstdout, csbi.wAttributes );

	if (LogFile.is_open())
	{
		std::stringstream prefix_ss;
		prefix_ss << std::put_time(&local_tm, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << now_ms.count();
		prefix_ss << " " << typeStr << ":";
		const int prefixWidth = 22;
		std::string prefix = prefix_ss.str();
		LogFile << std::left << std::setw(prefixWidth) << prefix << " " << Message << std::endl;

		LogFile.flush();
	}
}