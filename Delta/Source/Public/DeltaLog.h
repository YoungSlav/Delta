#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip> // for std::setw, std::setprecision
#include <type_traits> // for std::is_floating_point
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>
#include <stdexcept>

#define LOG(Type, ...) DeltaLog::logMessage(Type, __VA_ARGS__)
#define LOG_INDENT DeltaLogIndentScope indent;

enum ELog
{
	Success,
	Log,
	Warning,
	Error,
	Fatal
};



class DeltaLog
{
	friend class DeltaLogIndentScope;
public:

	static void init(const std::string& logFilename = "Delta.log");


	template<typename... Args>
	static void logMessage(ELog Type, const std::string& format_str, Args&&... args)
	{
		auto tuple_args = std::make_tuple(DeltaLog::toString(std::forward<Args>(args))...);
		auto format_args = std::apply(
			[&](auto&... args) { return std::make_format_args(args...); },
			tuple_args
		);
		print(std::vformat(format_str, format_args), Type);
	}

	template<typename T>
	static std::string toString(const T& val)
	{
		return toStringInternal<T>(val);
	}
private:

	static void increaseIndent();
	static void decreaseIndent();

	static void print(const std::string& Message, ELog Type = ELog::Log);
	static void print(const char* const Message, ELog Type = ELog::Log);

	template<typename T=char*>
	static std::string toStringInternal(const char* val) 
	{
		return std::string(val);
	}

	template<typename T=std::string>
	static std::string toStringInternal(const std::string& val) 
	{
		return val;
	}

    // GLM quaternion overload (use tquat to be version-agnostic)
    template<typename U, glm::qualifier Q = glm::qualifier::defaultp>
    static std::string toStringInternal(const glm::tquat<U, Q>& q)
    {
        return "(" + toStringInternal<glm::vec3>(glm::normalize(glm::vec3(q.x, q.y, q.z))) + " " + toStringInternal<float>(glm::degrees(q.w)) + ")";
    }
	
	
	// Templated function to convert glm vector types to string
	template<typename T, glm::length_t L, typename U = typename T::value_type>
	static std::string toStringInternal(const glm::vec<L, U>& vec) 
	{
		// Create a stringstream to build the string
		std::stringstream ss;

		// Set the width for each component
		const int componentWidth = 6;

		// Write each component of the vector to the stringstream
		ss << "(";
		for (int i = 0; i < L; ++i) 
		{
			if (i > 0) 
			{
				ss << " ";
			}
			// Set precision for floating-point types
			if constexpr (std::is_floating_point<U>::value) 
			{
				ss << std::setw(componentWidth) << std::fixed << std::setprecision(3) << std::left << vec[i];
			} 
			else 
			{
				ss << std::setw(componentWidth) << std::left << vec[i];
			}
		}
		ss << ")";

		// Convert the stringstream to a string and return it
		return ss.str();
	}

	// Templated function to convert glm matrix types to string
	template<typename T, glm::length_t C, glm::length_t R, typename U = typename T::value_type>
	static std::string toStringInternal(const glm::mat<C, R, U>& mat) 
	{
		std::string str = "(";
		for (int i = 0; i < C; ++i) 
		{
			if (i > 0) 
			{
				str += ", ";
			}
			str += toStringInternal<T>(mat[i]);
		}
		str += ")";
		return str;
	}

	// Templated function to convert non-glm types to string using std::to_string
	template<typename T>
	static std::string toStringInternal(const T& val) 
	{
		return std::to_string(val);
	}

private:
	static std::ofstream logFile;
	static std::string logFileName;
	static std::string logFolder;
	static int logIndent;
	static constexpr int indentSize = 4;

	static void renameOldLogFile(const std::string& oldFileName);
};

class DeltaLogIndentScope
{
public:
	DeltaLogIndentScope()
	{
		DeltaLog::increaseIndent();
	}
	~DeltaLogIndentScope()
	{
		DeltaLog::decreaseIndent();
	}
};
