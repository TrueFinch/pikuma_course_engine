//
// Created by Vladimir Glushkov on 24.07.2026.
//

#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <fmt/ranges.h>

template<glm::length_t L, typename T, glm::qualifier Q>
struct fmt::formatter<glm::vec<L, T, Q>> {
	static constexpr auto parse(const format_parse_context& ctx) {
		return ctx.begin();
	}

	static auto format(const glm::vec<L, T, Q>& v, format_context& ctx) {
		auto begin = glm::value_ptr(v);
		auto end = begin + L;
		return fmt::format_to(ctx.out(), "{{{}}}", fmt::join(begin, end, ", "));
	}
};
