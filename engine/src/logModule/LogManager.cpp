//
// Created by Vladimir Glushkov on 08.01.2025.
//

#include "engine/logModule/LogManager.h"

#include <memory>
#include <ranges>
#include <vector>

using namespace pce::logModule;

std::unique_ptr<LogManager> LogManager::Create() {
	auto logManager = std::unique_ptr<LogManager>(new LogManager());
	return logManager;
}

void LogManager::Log(eLogLevel lvl, const std::string_view msg) const {
	for (const auto &obs : m_observers) {
		obs->OnLog(lvl, msg);
	}
}

void LogManager::RegisterObserver(std::unique_ptr<LogObserver> obs) {
	m_observers.push_back(std::move(obs));
	m_observers.back()->OnRegistered();
}

void LogManager::UnregisterObserver(const LogObserver::ID id) {
	const auto it = std::ranges::find_if(m_observers, [id](const std::unique_ptr<LogObserver> &obs) {
		return obs->m_id == id;
	});
	(*it)->OnUnregistered();
	m_observers.erase(it);
}

void LogManager::Teardown() {
	while (!m_observers.empty()) {
		UnregisterObserver(m_observers.front()->m_id);
	}
}
