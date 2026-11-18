
#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

enum class ResultadoCiclo {
    PrimeiraAquisicao,
    NovoCiclo,
    CicloRepetido,
    CicloAntigo,
    NovaSessao
};

class EstadoAquisicao {

public:

    ResultadoCiclo avaliar(
        const std::string& sessao,
        std::uint64_t ciclo) const;

    void registrar(
        const std::string& sessao,
        std::uint64_t ciclo);

    void carregar(
        const std::filesystem::path& caminho);

    void salvar(
        const std::filesystem::path& caminho) const;

private:

    struct Registro {
        std::string sessao;
        std::uint64_t ciclo;
    };

    std::optional<Registro> ultimoRegistro_;
};
