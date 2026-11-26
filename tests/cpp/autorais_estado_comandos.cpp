#include "EstadoAquisicao.hpp"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {

struct DiretorioTeste {
    DiretorioTeste() {
        char modelo[] = "/tmp/poo-estado-comando-XXXXXX";
        const char* criado = mkdtemp(modelo);
        if (!criado) {
            throw std::runtime_error("Falha ao criar diretorio de teste");
        }
        caminho = criado;
    }

    ~DiretorioTeste() {
        std::error_code erro;
        std::filesystem::remove_all(caminho, erro);
    }

    std::filesystem::path caminho;
};

template <typename Erro, typename Operacao>
void deveFalhar(Operacao operacao) {
    bool falhou = false;
    try {
        operacao();
    } catch (const Erro&) {
        falhou = true;
    }
    assert(falhou);
}

void verificarPendente(const EstadoAquisicao& estado) {
    const auto pedido = estado.pendente();
    assert(pedido);
    assert(pedido->ciclo == 40);
    assert(pedido->comando.sessao() == "sessao-A");
    assert(pedido->comando.id() == 12);
    assert(pedido->comando.atuacao().bomba() == 70.5);
    assert(pedido->comando.atuacao().aquecedor() == 25.0);
    assert(estado.ultimoComandoConfirmado() == 11);
}

}

int main() {
    const DiretorioTeste diretorio;
    const auto caminho = diretorio.caminho / "controlador_estado.json";
    const auto comando = diretorio.caminho / "comando_planta.json";
    const auto invalido = diretorio.caminho / "invalido.json";

    std::ofstream(caminho) << R"({"sessao":"sessao-A","ciclo":39})";
    {
        EstadoAquisicao estado;
        estado.carregar(caminho);
        assert(estado.avaliar("sessao-A", 39) == ResultadoCiclo::CicloRepetido);
        assert(!estado.pendente());
        estado.reconciliar("sessao-A", 40, 11);
        assert(estado.reservarComando("sessao-A", 40, Atuacao(70.5, 25)).id() == 12);
        estado.salvar(caminho);
        // Encerra aqui, com a reserva salva e sem publicar o comando.
    }

    assert(!std::filesystem::exists(comando));
    {
        EstadoAquisicao estado;
        estado.carregar(caminho);
        verificarPendente(estado);
        assert(estado.avaliar("sessao-A", 40) == ResultadoCiclo::NovoCiclo);
        deveFalhar<std::logic_error>([&] {
            estado.reservarComando("sessao-A", 40, Atuacao(0, 0));
        });
        estado.pendente()->comando.publicar(comando);
        // Encerra depois de publicar, antes de registrar o ciclo.
    }

    assert(std::filesystem::exists(comando));
    EstadoAquisicao estado;
    estado.carregar(caminho);
    estado.reconciliar("sessao-A", 41, 11);
    verificarPendente(estado);
    deveFalhar<std::logic_error>([&] {
        estado.reservarComando("sessao-A", 41, Atuacao(0, 0));
    });

    deveFalhar<std::invalid_argument>([&] { estado.reconciliar("sessao-A", 41, 10); });
    deveFalhar<std::invalid_argument>([&] { estado.reconciliar("sessao-A", 0, 11); });
    deveFalhar<std::invalid_argument>([&] { estado.reconciliar("sessao-A", 38, 11); });
    deveFalhar<std::invalid_argument>([&] { estado.reconciliar("sessao-A", 40, 12); });
    verificarPendente(estado);

    for (const char* json : {
        R"({"sessao":"sessao-A","ciclo":39,"ultimo_comando_confirmado":true})",
        R"({"sessao":"sessao-A","ciclo":39,"ultimo_comando_confirmado":11,"solicitacao":{"ciclo":40,"id":13,"bomba":50,"aquecedor":25}})",
        R"({"sessao":"sessao-A","ciclo":39,"ultimo_comando_confirmado":11,"solicitacao":{"ciclo":40,"id":12,"bomba":101,"aquecedor":25}})",
        R"({"sessao":"sessao-A","ciclo":39,"solicitacao":{"ciclo":40,"id":12,"bomba":50,"aquecedor":25}})"
    }) {
        std::ofstream(invalido) << json;
        deveFalhar<std::runtime_error>([&] { estado.carregar(invalido); });
        verificarPendente(estado);
    }

    const auto temporario = diretorio.caminho / "controlador_estado.json.tmp";
    std::filesystem::create_directory(temporario);
    estado.reconciliar("sessao-A", 42, 12);
    assert(!estado.pendente());
    assert(estado.ultimoComandoConfirmado() == 12);
    deveFalhar<std::runtime_error>([&] { estado.salvar(caminho); });
    EstadoAquisicao recuperado;
    recuperado.carregar(caminho);
    verificarPendente(recuperado);
    assert(std::filesystem::is_directory(temporario));

    std::filesystem::remove(temporario);
    estado.salvar(caminho);
    recuperado.carregar(caminho);
    assert(!recuperado.pendente());
    assert(recuperado.ultimoComandoConfirmado() == 12);
    deveFalhar<std::logic_error>([&] {
        recuperado.reservarComando("sessao-A", 40, Atuacao(0, 0));
    });
    assert(recuperado.reservarComando("sessao-A", 42, Atuacao(0, 0)).id() == 13);
    recuperado.reconciliar("sessao-A", 43, 14);
    assert(!recuperado.pendente());
    assert(recuperado.ultimoComandoConfirmado() == 14);

    recuperado.reconciliar("sessao-B", 1, 0);
    assert(!recuperado.pendente());
    assert(recuperado.avaliar("sessao-B", 1) == ResultadoCiclo::PrimeiraAquisicao);
    assert(recuperado.reservarComando("sessao-B", 1, Atuacao(0, 100)).id() == 1);
    recuperado.salvar(caminho);
    estado.carregar(caminho);
    assert(estado.pendente()->comando.id() == 1);
    assert(estado.avaliar("sessao-B", 1) == ResultadoCiclo::PrimeiraAquisicao);
    estado.reconciliar("sessao-C", 1, std::numeric_limits<std::int64_t>::max());
    deveFalhar<std::overflow_error>([&] {
        estado.reservarComando("sessao-C", 1, Atuacao(0, 0));
    });
    assert(!estado.pendente());

    std::cout << "Testes autorais de persistencia e reconciliacao de comandos aprovados!\n";
}
