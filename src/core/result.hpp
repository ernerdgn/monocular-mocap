#pragma once
#include <string>
#include <variant>
#include <stdexcept>
#include <type_traits>

namespace mocap {

    template <typename T>
    class Result
    {
        struct Error { std::string msg; };
        std::variant<T, Error> m_data;

        Result(Error err) : m_data(std::move(err)) {}

        public:
        static Result<T> ok(T value) { return Result<T>(std::move(value)); }
        static Result<T> err(std::string error_msg) { return Result<T>(Error{std::move(error_msg)}); }

        Result(const T& val) : m_data(val) {}
        Result(T&& val) : m_data(std::move(val)) {}

        template <typename U = T, typename = std::enable_if_t<!std::is_same_v<U, std::string>>>
        Result(std::string err_msg) : m_data(Error{std::move(err_msg)}) {}

        template <typename U = T, typename = std::enable_if_t<!std::is_same_v<U, std::string>>>
        Result(const char* err_msg) : m_data(Error{std::string(err_msg)}) {}

        bool is_ok() const { return std::holds_alternative<T>(m_data); }
        bool is_err() const { return std::holds_alternative<Error>(m_data); }

        const T& value() const { return std::get<T>(m_data); }
        const std::string& error() const { return std::get<Error>(m_data).msg; }

        template <typename Func>
        auto map(Func&& f) const -> Result<decltype(f(std::declval<T>()))>
        {
            using R = decltype(f(std::declval<T>()));
            if (is_ok()) return Result<R>::ok(f(value()));
            return Result<R>::err(error());
        }

        template <typename Func>
        Result<T> map_error(Func&& f) const
        {
            if (is_err()) return Result<T>::err(f(error()));
            return Result<T>::ok(value());
        }
    };

    template <>
    class Result<void>
    {
        bool m_is_ok;
        std::string m_error;

        Result(bool ok, std::string err) : m_is_ok(ok), m_error(std::move(err)) {}
        public:
        static Result<void> ok() { return Result<void>(true, ""); }
        static Result<void> err(std::string error_msg) { return Result<void>(false, std::move(error_msg)); }
        static Result<void> fail(std::string error_msg) { return err(std::move(error_msg)); } // Alias for old code

        bool is_ok() const { return m_is_ok; }
        bool is_err() const { return !m_is_ok; }

        const std::string& error() const { return m_error; }

        Result() : m_is_ok(true) {}
        Result(const char* err_msg) : m_is_ok(false), m_error(err_msg) {}
        Result(std::string err_msg) : m_is_ok(false), m_error(std::move(err_msg)) {}
    };

    using VoidResult = Result<void>;

}