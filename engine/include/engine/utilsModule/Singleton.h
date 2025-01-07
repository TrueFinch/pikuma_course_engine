//
// Created by Vladimir Glushkov on 08.01.2025.
//

#pragma once

#include <memory>

namespace pce::utilsModule {
	template<typename T>
	class Singleton {
	public:
		Singleton() = delete;

		static T &GetInstance() {
			return *m_instance;
		}

		static void Init(std::unique_ptr<T> instance) {
			m_instance = std::move(instance);
		}

		static void Cleanup() {
			m_instance.reset();
		}

	protected:
		static inline std::unique_ptr<T> m_instance;
	};
}
