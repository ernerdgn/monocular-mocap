#pragma once
#include <variant>
#include <string>

namespace mocap {

    template <typename T>
    class [[nodiscard]] Result
    {
        public:
        Result(T value) : m_data(std::move(value)) {}
        Result(const char* error) : m_data(std::string(error)) {}
        Result(std::string error) : m_data(std::move(error)) {}

        bool is_ok() const { return std::holds_alternative<T>(m_data); }
        const T& value() const { return std::get<T>(m_data); }
        const std::string& error() const {return std::get<std::string>(m_data); }

        private:
        std::variant<T, std::string> m_data;
    };

    template <>
    class [[nodiscard]] Result<void>
    {
        public:
        static Result<void> ok() { return Result<void>(true); }
        static Result<void> fail(std::string err) { return Result<void>(std::move(err)); }

        bool is_ok() const { return m_isOk; }
        const std::string& error() const { return m_error; }

        private:
        Result(bool ok) : m_isOk(ok) {}
        Result(std::string err) : m_error(std::move(err)), m_isOk(false) {}
        bool m_isOk;
        std::string m_error;
    };
}