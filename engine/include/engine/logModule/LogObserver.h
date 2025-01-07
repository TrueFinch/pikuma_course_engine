//
// Created by Vladimir Glushkov on 08.01.2025.
//

#pragma once

#include "engine/utilsModule/types.h"

namespace pce::logModule {
	class LogObserver {
		friend class LogManager;

	public:
		using ID = uint32;

		LogObserver() {
			m_id = m_index++;
		}

		bool operator==(const LogObserver &other) const {
			return m_id == other.m_id;
		}

		virtual ~LogObserver() = default;

		virtual void OnRegistered() = 0;

		virtual void OnUnregistered() = 0;

		virtual void OnLog(eLogLevel lvl, std::string_view msg) = 0;

	private:
		static inline uint32 m_index = 0;
		ID m_id;
	};
}
