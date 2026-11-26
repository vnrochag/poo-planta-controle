
#pragma once

#include "Comando.hpp"

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

    struct Solicitacao {
        std::uint64_t ciclo;
        Comando comando;
    };

    void reconciliar(const std::string& sessao, std::uint64_t ciclo,
                     std::uint64_t ultimoComando);
    Comando reservarComando(const std::string& sessao, std::uint64_t ciclo,
                            const Atuacao& atuacao);
    std::optional<Solicitacao> pendente() const;
    std::uint64_t ultimoComandoConfirmado() const;

private:

    struct Registro {
        std::string sessao;
        std::uint64_t ciclo;
        std::uint64_t confirmado = 0;
        std::optional<Solicitacao> solicitacao = std::nullopt;
    };

    std::optional<Registro> ultimoRegistro_;
};
