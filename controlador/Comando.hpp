#pragma once

#include "Atuacao.hpp"

#include <cstdint>
#include <filesystem>
#include <string>

class Comando {
public:
    Comando(std::string sessao, std::int64_t id, Atuacao atuacao);

    const std::string& sessao() const;
    std::int64_t id() const;
    const Atuacao& atuacao() const;

    void publicar(const std::filesystem::path& caminho) const;

private:
    std::string sessao_;
    std::int64_t id_;
    Atuacao atuacao_;
};
