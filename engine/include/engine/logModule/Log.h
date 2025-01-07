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
	void log(const std::string_view msg, Args &&... args) {
		log(eLogLevel::PCE_LOG, fmt::format(msg, args...));
	}

	void log(std::string_view msg);

	template<typename... Args>
	void logWarning(const std::string_view msg, Args &&... args) {
		log(eLogLevel::PCE_WARNING, fmt::format(fmt::runtime(msg), std::forward<Args>(args)...));
	}

	inline void logWarning(std::string_view msg);

	template<typename... Args>
	void logError(const std::string_view msg, Args &&... args) {
		log(eLogLevel::PCE_ERROR, fmt::format(fmt::runtime(msg), std::forward<Args>(args)...));
	}

	inline void logError(std::string_view msg);
}
