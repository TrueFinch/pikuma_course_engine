#pragma once

#include <iostream>
#include <cstdlib>
#include <string>
#include <stdexcept>

#include "engine/logModule/Log.h"
#if !defined(PCE_TESTING) && !defined(CATCH_VERSION_MAJOR) && !defined(CATCH_CONFIG_PREFIX_ALL)
#include <SDL2/SDL.h>
#endif

// Cross-platform hardware breakpoint
#if defined(_MSC_VER)
#define PCE_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#if defined(__i386__) || defined(__x86_64__)
#define PCE_DEBUG_BREAK() __asm__ volatile("int $3")
#elif defined(__aarch64__)
#define PCE_DEBUG_BREAK() __asm__ volatile("brk #0xf000")
#elif defined(__arm__)
#define PCE_DEBUG_BREAK() __asm__ volatile("bkpt #0")
#else
#include <signal.h>
#define PCE_DEBUG_BREAK() raise(SIGTRAP)
#endif
#else
#define PCE_DEBUG_BREAK() ((void)0)
#endif

namespace pce {
	class AssertionException: public std::runtime_error {
	public:
		explicit AssertionException(const std::string& message)
			: std::runtime_error(message) {}

		explicit AssertionException(const char* message)
			: std::runtime_error(message) {}
	};

	namespace internal {
		inline bool& GetThrowOnAssertRef() {
			static bool throwOnAssert = false;
			return throwOnAssert;
		}

		inline void SetThrowOnAssert(bool value) {
			GetThrowOnAssertRef() = value;
		}

		inline bool IsThrowOnAssert() {
#if defined(PCE_TESTING) || defined(CATCH_VERSION_MAJOR) || defined(CATCH_CONFIG_PREFIX_ALL)
			return true;
#else
			return GetThrowOnAssertRef();
#endif
		}

#if !defined(PCE_TESTING) && !defined(CATCH_VERSION_MAJOR) && !defined(CATCH_CONFIG_PREFIX_ALL)
		inline int ShowAssertWindow(const char* conditionStr, const char* message, const char* file, int line) {
			constexpr SDL_MessageBoxButtonData buttons[] = {
				{SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Abort"},
				{SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Debug"},
				{0, 2, "Ignore"}
			};

			std::string fullMessage = "Condition: " + std::string(conditionStr) + "\n";
			if (message && message[0] != '\0') {
				fullMessage += "Message: " + std::string(message) + "\n";
			}
			fullMessage += "\nFile: " + std::string(file) +
					"\nLine: " + std::to_string(line) +
					"\n\nActions:" +
					"\n[Ignore] - Skip & continue" +
					"\n[Debug] - Pause & open IDE" +
					"\n[Abort] - Exit game";

			const SDL_MessageBoxData messageBoxData = {
				SDL_MESSAGEBOX_ERROR,
				nullptr,
				"Assertion Failed!",
				fullMessage.c_str(),
				std::size(buttons),
				buttons,
				nullptr
			};

			int buttonId = -1;
			SDL_ShowMessageBox(&messageBoxData, &buttonId);
			return buttonId;
		}
#else
		// Stub implementation for test builds – just return "Ignore" (2)
		inline int ShowAssertWindow(const char* /*conditionStr*/, const char* /*message*/, const char* /*file*/,
									int /*line*/) {
			return 2; // corresponds to "Ignore"
		}
#endif

		inline void AssertHandler(bool condition, const char* conditionStr, const char* message, const char* file,
								int line) {
			if (!condition) {
				if (IsThrowOnAssert()) {
					throw pce::AssertionException(
						std::string("Assertion Failed: ") + conditionStr +
						((message && message[0] != '\0') ? std::string(" (") + std::string(message) + ")" : "")
					);
				}
				pce::logError("[ASSERT FAILED]\nCondition: {}\nMessage: {}\nFile: {}\nLine: {}",
							conditionStr,
							(message ? message : ""),
							file,
							line);

				int result = ShowAssertWindow(conditionStr, message, file, line);

				if (result == 0) {
					std::abort();
				}
				if (result == 1) {
					PCE_DEBUG_BREAK();
				}
			}
		}
	}

	inline void SetThrowOnAssert(bool value) {
		internal::SetThrowOnAssert(value);
	}
}

#if defined(NDEBUG)
// In regular release builds, the macro is cut out completely
#define PCE_ASSERT(condition, message) ((void)0)
#else
// Expands to a function call expression (type void) so it can be passed inside Catch2 macros like REQUIRE_THROWS_AS
#define PCE_ASSERT(condition, message) \
	::pce::internal::AssertHandler(static_cast<bool>(condition), #condition, message, __FILE__, __LINE__)
#endif
