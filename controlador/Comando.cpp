#include "Comando.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>
#include <system_error>
#include <utility>

Comando::Comando(std::string sessao, std::int64_t id, Atuacao atuacao)
    : sessao_(std::move(sessao)), id_(id), atuacao_(atuacao) {
    if (sessao_.empty()) {
        throw std::invalid_argument("Sessao do comando deve ser nao vazia");
    }
    if (id_ <= 0) {
        throw std::invalid_argument("ID do comando deve ser positivo");
    }
}

const std::string& Comando::sessao() const {
    return sessao_;
}

std::int64_t Comando::id() const {
    return id_;
}

const Atuacao& Comando::atuacao() const {
    return atuacao_;
}

void Comando::publicar(const std::filesystem::path& caminho) const {
    const nlohmann::json documento = {
        {"versao", 1},
        {"sessao", sessao_},
        {"id", id_},
        {"atuadores", {
            {"bomba", atuacao_.bomba()},
            {"aquecedor", atuacao_.aquecedor()}
        }}
    };
    const std::string conteudo = documento.dump(4);

    std::filesystem::path temporario = caminho;
    temporario += ".tmp";
    std::ofstream arquivo(temporario, std::ios::binary);
    if (!arquivo.is_open()) {
        throw std::runtime_error("Nao foi possivel abrir o comando temporario");
    }

    try {
        arquivo << conteudo << '\n';
        arquivo.close();
        if (!arquivo) {
            throw std::runtime_error("Falha ao gravar o comando");
        }
        std::filesystem::rename(temporario, caminho);
    } catch (...) {
        std::error_code erro;
        std::filesystem::remove(temporario, erro);
        throw;
    }
}
