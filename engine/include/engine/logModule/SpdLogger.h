//
// Created by Vladimir Glushkov on 08.01.2025.
//

#pragma once

#include <string_view>

#include "engine/logModule/LogObserver.h"
#include "engine/utilsModule/Types.h"

namespace pce::logModule {
	class SpdLogger final : public LogObserver {
	public:
		explicit SpdLogger() = default;

		~SpdLogger() override = default;

		void OnRegistered() override;

		void OnUnregistered() override;

		void OnLog(eLogLevel lvl, std::string_view msg) override;
	};
}
