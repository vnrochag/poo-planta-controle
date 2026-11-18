
#pragma once

#include "Medicao.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

struct DadosPlanta {
    int versao;
    std::string sessao;
    std::uint64_t ciclo;
    std::string timestamp;

    Medicao nivel;
    Medicao temperatura;

    double bombaAplicada;
    double aquecedorAplicado;

    std::uint64_t ultimoComando;
    std::optional<std::string> erroComando;
    std::optional<std::string> motivoSeguro;
};

class Aquisicao {
public:
    static DadosPlanta lerArquivo(
        const std::filesystem::path& caminho);
};
