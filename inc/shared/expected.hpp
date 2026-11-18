/**
 * @file expected.hpp
 * @author Ryan Butler
 * @brief Wrapper around std::expected for Traffic SImulation project
 * @version 0.1
 * @date 2026-10-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once

#include <expected>
#include <string>
#include <format>

namespace Traffic {

struct Error {
    std::string message;
    int code = 0;
};

template <typename R>
using Expected = std::expected<R, Error>;

template <typename R>
Expected<R> make_expected(R& value){
    return value;
}

template <typename... Args>
std::unexpected<Error> make_error(int code, std::format_string<Args...> fmt, Args&&... args){
    return std::unexpected(Error{std::format(fmt, std::forward<Args>(args)...), code});
}

}; // namespace Traffic