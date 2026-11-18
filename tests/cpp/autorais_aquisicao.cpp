
#include "Aquisicao.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

int main() {

    // Caminho do arquivo usado somente para o teste.
    const std::filesystem::path caminho =
        "build/teste_aquisicao.json";

    // Cria um JSON de teste com dados validos.
    std::ofstream arquivo(caminho);

    arquivo << R"({
        "versao": 1,
        "sessao": "sessao-teste",
        "ciclo": 12,
        "timestamp": "2026-11-17T15:30:00Z",
        "leituras": [
            {
                "tag": "LT-01",
                "tipo": "nivel",
                "valor": 48.5,
                "unidade": "%",
                "status": "operando"
            },
            {
                "tag": "TT-01",
                "tipo": "temperatura",
                "valor": 26.2,
                "unidade": "C",
                "status": "operando"
            }
        ],
        "atuadores": {
            "bomba": 75.0,
            "aquecedor": 20.0
        },
        "ultimo_comando": 1,
        "erro_comando": null,
        "motivo_seguro": null
    })";

    // Fecha o arquivo antes de iniciar a leitura.
    arquivo.close();

    // Chama o metodo que estamos testando.
    DadosPlanta dados = Aquisicao::lerArquivo(caminho);

    // Verifica as informacoes da aquisicao.
    assert(dados.versao == 1);
    assert(dados.sessao == "sessao-teste");
    assert(dados.ciclo == 12);

    // Verifica os sensores.
    assert(dados.nivel.valor().value() == 48.5);
    assert(dados.temperatura.valor().value() == 26.2);

    // Verifica as potencias aplicadas.
    assert(dados.bombaAplicada == 75.0);
    assert(dados.aquecedorAplicado == 20.0);

    // Verifica a confirmacao do comando.
    assert(dados.ultimoComando == 1);

    // Verifica os campos opcionais.
    assert(!dados.erroComando.has_value());
    assert(!dados.motivoSeguro.has_value());

    // Remove o arquivo temporario do teste.
    std::filesystem::remove(caminho);

    std::cout << "Teste de aquisicao valida aprovado!\n";

    return 0;
}
