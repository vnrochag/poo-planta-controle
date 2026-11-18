#pragma once

#include <optional>
#include <string>

class Medicao {
public:
    Medicao(std::string tag,
            std::string tipo,
            std::optional<double> valor,
            std::string unidade,
            std::string status);

    const std::string& tag() const;
    const std::string& tipo() const;
    const std::optional<double>& valor() const;
    const std::string& unidade() const;
    const std::string& status() const;

    bool estaOperando() const;

private:
    std::string tag_;
    std::string tipo_;
    std::optional<double> valor_;
    std::string unidade_;
    std::string status_;
};
