//
// Created by Vladimir Glushkov on 08.01.2025.
//

#pragma once

#include <string>
#include <fmt/format.h>

#include "engine/utilsModule/Types.h"

namespace pce {
	void log(eLogLevel lvl, std::string_view msg);

	template<typename... Args>
	void log(fmt::format_string<Args...> msg, Args&&... args) {
		log(eLogLevel::PCE_LOG, fmt::vformat(msg, fmt::make_format_args(args...)));
	}

	void log(std::string_view msg);

	template<typename... Args>
	void logWarning(fmt::format_string<Args...> msg, Args&&... args) {
		log(eLogLevel::PCE_WARNING, fmt::vformat(msg, fmt::make_format_args(args...)));
	}

	inline void logWarning(std::string_view msg);

	template<typename... Args>
	void logError(fmt::format_string<Args...> msg, Args&&... args) {
		log(eLogLevel::PCE_ERROR, fmt::vformat(msg, fmt::make_format_args(args...)));
	}

	inline void logError(std::string_view msg);
}
