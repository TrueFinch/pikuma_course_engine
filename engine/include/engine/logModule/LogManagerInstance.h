//
// Created by Mytona on 08.01.2025.
//

#pragma once

#include "engine/utilsModule/Singleton.h"

namespace pce::logModule {
	class LogManager;

	class LogManagerInstance final : public utilsModule::Singleton<LogManager> {};
}
