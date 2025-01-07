//
// Created by Vladimir Glushkov on 08.01.2025.
//

#pragma once

#include <list>
#include <memory>
#include <fmt/format.h>

#include "engine/logModule/LogObserver.h"
#include "engine/utilsModule/Types.h"

namespace pce::logModule {
	class LogManager final {
	public:
		static std::unique_ptr<LogManager> Create();

		~LogManager() = default;

		LogManager(const LogManager &) = delete;

		LogManager(LogManager &&) = delete;

		LogManager &operator=(const LogManager &) = delete;

		LogManager &operator=(LogManager &&) = delete;

		void Log(eLogLevel lvl, std::string_view msg) const;

		void RegisterObserver(std::unique_ptr<LogObserver> obs);

		void UnregisterObserver(LogObserver::ID id);

		void Teardown();

	protected:
		LogManager() = default;

	private:
		std::list<std::unique_ptr<LogObserver> > m_observers;
	};
}
