#include "Comando.hpp"

#include <nlohmann/json.hpp>

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {

class DiretorioTeste {
public:
    DiretorioTeste() {
        char modelo[] = "/tmp/poo-comando-XXXXXX";
        const char* criado = mkdtemp(modelo);
        if (criado == nullptr) {
            throw std::runtime_error("Nao foi possivel criar o diretorio de teste");
        }
        caminho = criado;
    }

    ~DiretorioTeste() {
        std::error_code erro;
        std::filesystem::remove_all(caminho, erro);
    }

    std::filesystem::path caminho;
};

void deveRejeitar(const std::string& sessao, std::int64_t id) {
    bool rejeitou = false;
    try {
        const Comando comando(sessao, id, Atuacao(50.0, 50.0));
    } catch (const std::invalid_argument&) {
        rejeitou = true;
    }
    assert(rejeitou);
}

nlohmann::json ler(const std::filesystem::path& caminho) {
    std::ifstream arquivo(caminho);
    return nlohmann::json::parse(arquivo);
}

}

int main() {
    const DiretorioTeste diretorio;
    const auto destino = diretorio.caminho / "comando_planta.json";
    const auto temporario = diretorio.caminho / "comando_planta.json.tmp";

    deveRejeitar("", 1);
    deveRejeitar("sessao-teste", 0);
    deveRejeitar("sessao-teste", -1);

    const std::string sessao = u8"sess\u00e3o \"A\"";
    const Comando primeiro(sessao, 1, Atuacao(0.0, 100.0));
    assert(primeiro.sessao() == sessao);
    assert(primeiro.id() == 1);
    assert(primeiro.atuacao().bomba() == 0.0);
    assert(primeiro.atuacao().aquecedor() == 100.0);
    primeiro.publicar(destino);

    const nlohmann::json esperado = {
        {"versao", 1}, {"sessao", sessao}, {"id", 1},
        {"atuadores", {{"bomba", 0.0}, {"aquecedor", 100.0}}}
    };
    const auto publicado = ler(destino);
    assert(publicado == esperado);
    assert(publicado.at("versao").is_number_integer());
    assert(publicado.at("id").is_number_integer());
    assert(!std::filesystem::exists(temporario));

    const auto maiorId = std::numeric_limits<std::int64_t>::max();
    const Comando segundo(sessao, maiorId, Atuacao(70.5, 25.0));
    segundo.publicar(destino);
    const auto substituto = ler(destino);
    assert(substituto.at("id").get<std::int64_t>() == maiorId);
    assert(substituto.at("atuadores").at("bomba") == 70.5);
    assert(substituto.at("atuadores").at("aquecedor") == 25.0);

    // Um diretorio no caminho temporario impede a abertura para escrita.
    std::filesystem::create_directory(temporario);
    bool falhou = false;
    try {
        primeiro.publicar(destino);
    } catch (const std::runtime_error&) {
        falhou = true;
    }
    assert(falhou);
    assert(ler(destino) == substituto);
    assert(std::filesystem::is_directory(temporario));

    // Renomear um arquivo sobre um diretorio falha e limpa o temporario.
    const auto bloqueado = diretorio.caminho / "destino-bloqueado";
    std::filesystem::create_directory(bloqueado);
    falhou = false;
    try {
        primeiro.publicar(bloqueado);
    } catch (const std::filesystem::filesystem_error&) {
        falhou = true;
    }
    assert(falhou);
    assert(std::filesystem::is_directory(bloqueado));
    assert(!std::filesystem::exists(diretorio.caminho / "destino-bloqueado.tmp"));

    std::cout << "Testes autorais de Comando aprovados!\n";
    return 0;
}
