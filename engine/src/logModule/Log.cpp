//
// Created by Vladimir Glushkov on 08.01.2025.
//

#include "engine/logModule/Log.h"

#include "engine/logModule/LogManager.h"
#include "engine/logModule/LogManagerInstance.h"

void pce::log(eLogLevel lvl, const std::string_view msg) {
	using namespace logModule;
	const auto &logger = LogManagerInstance::GetInstance();
	logger.Log(lvl, msg);
}

void pce::log(const std::string_view msg) {
	log(eLogLevel::PCE_LOG, msg);
}

void pce::logWarning(const std::string_view msg) {
	log(eLogLevel::PCE_WARNING, msg);
}

void pce::logError(const std::string_view msg) {
	log(eLogLevel::PCE_ERROR, msg);
}
