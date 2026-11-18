
#include "Aquisicao.hpp"

#include <nlohmann/json.hpp>

#include <cassert>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

// Verifica se a aquisicao rejeita um arquivo
// com a mensagem de erro esperada.
void deveFalhar(
    const std::filesystem::path& caminho,
    const std::string& mensagemEsperada) {

    bool encontrouErro = false;

    try {
        Aquisicao::lerArquivo(caminho);
    } catch (const std::exception& erro) {
        encontrouErro =
            std::string(erro.what()).find(mensagemEsperada)
            != std::string::npos;
    }

    assert(encontrouErro);
}

int main() {

    // TESTE 1: arquivo inexistente.
    const std::filesystem::path ausente =
        "build/teste_aquisicao_ausente.json";

    std::filesystem::remove(ausente);

    deveFalhar(
        ausente,
        "Nao foi possivel abrir o arquivo");

    // Caminho utilizado nos demais testes.
    const std::filesystem::path caminho =
        "build/teste_aquisicao_falhas.json";

    // TESTE 2: JSON malformado.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({"versao": 1, "sessao": )";
    }

    deveFalhar(
        caminho,
        "Arquivo JSON invalido");

    // TESTE 3: JSON com raiz incorreta.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"([1, 2, 3])";
    }

    deveFalhar(
        caminho,
        "JSON deve conter um objeto");

    // TESTE 4: versao diferente de 1.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({"versao": 2})";
    }

    deveFalhar(
        caminho,
        "Versao do contrato nao suportada");

    // TESTE 5: versao booleana.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({"versao": true})";
    }

    deveFalhar(
        caminho,
        "Versao deve ser um inteiro");

    // TESTE 6: campo versao ausente.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({"sessao": "teste"})";
    }

    deveFalhar(
        caminho,
        "Campo versao ausente");

    // TESTE 7: sessao vazia.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({
            "versao": 1,
            "sessao": ""
        })";
    }

    deveFalhar(
        caminho,
        "Sessao nao pode ser vazia");

    // TESTE 8: sessao com tipo incorreto.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({
            "versao": 1,
            "sessao": 123
        })";
    }

    deveFalhar(
        caminho,
        "Sessao ausente ou invalida");

    // TESTE 9: ciclo igual a zero.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({
            "versao": 1,
            "sessao": "sessao-teste",
            "ciclo": 0
        })";
    }

    deveFalhar(
        caminho,
        "Ciclo deve ser positivo");

    // TESTE 10: ciclo negativo.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({
            "versao": 1,
            "sessao": "sessao-teste",
            "ciclo": -3
        })";
    }

    deveFalhar(
        caminho,
        "Ciclo deve ser positivo");

    // TESTE 11: ciclo decimal.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({
            "versao": 1,
            "sessao": "sessao-teste",
            "ciclo": 2.5
        })";
    }

    deveFalhar(
        caminho,
        "Ciclo ausente ou invalido");

    // TESTE 12: formato de timestamp incorreto.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({
            "versao": 1,
            "sessao": "sessao-teste",
            "ciclo": 12,
            "timestamp": "2026-11-17 15:30:00Z"
        })";
    }

    deveFalhar(
        caminho,
        "Formato do timestamp invalido");

    // TESTE 13: data inexistente.
    // O ano de 2026 nao e bissexto.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({
            "versao": 1,
            "sessao": "sessao-teste",
            "ciclo": 12,
            "timestamp": "2026-02-29T12:00:00Z"
        })";
    }

    deveFalhar(
        caminho,
        "Dia invalido no timestamp");

    // TESTE 14: horario invalido.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({
            "versao": 1,
            "sessao": "sessao-teste",
            "ciclo": 12,
            "timestamp": "2026-11-17T25:00:00Z"
        })";
    }

    deveFalhar(
        caminho,
        "Data ou horario invalido");

    // TESTE 15: data valida em ano bissexto.
    // O timestamp deve ser aceito e a validacao
    // deve chegar ate o campo leituras.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

        arquivo << R"({
            "versao": 1,
            "sessao": "sessao-teste",
            "ciclo": 12,
            "timestamp": "2028-02-29T12:00:00Z"
        })";
    }

    deveFalhar(
        caminho,
        "Campo leituras ausente");

    // TESTE 16: sensor TT-01 ausente.
    // A planta fornece apenas o sensor LT-01.
    {
        std::ofstream arquivo(caminho);
        assert(arquivo.is_open());

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
                }
            ],
            "atuadores": {
                "bomba": 0.0,
                "aquecedor": 0.0
            },
            "ultimo_comando": 0,
            "erro_comando": null,
            "motivo_seguro": null
        })";
    }

    deveFalhar(
        caminho,
        "Sensor obrigatorio ausente");

    // Documento valido utilizado como base
    // para os testes 17 a 27.
    const nlohmann::json documentoValido = {
        {"versao", 1},
        {"sessao", "sessao-teste"},
        {"ciclo", 12},
        {"timestamp", "2026-11-17T15:30:00Z"},
        {"leituras", nlohmann::json::array({
            {
                {"tag", "LT-01"},
                {"tipo", "nivel"},
                {"valor", 48.5},
                {"unidade", "%"},
                {"status", "operando"}
            },
            {
                {"tag", "TT-01"},
                {"tipo", "temperatura"},
                {"valor", 26.2},
                {"unidade", "C"},
                {"status", "operando"}
            }
        })},
        {"atuadores", {
            {"bomba", 0.0},
            {"aquecedor", 0.0}
        }},
        {"ultimo_comando", 0},
        {"erro_comando", nullptr},
        {"motivo_seguro", nullptr}
    };

    // TESTE 17: sensor LT-01 duplicado.
    {
        nlohmann::json documento = documentoValido;

        // Adiciona novamente o primeiro sensor.
        documento["leituras"].push_back(
            documento["leituras"][0]);

        {
            std::ofstream arquivo(caminho);
            assert(arquivo.is_open());

            arquivo << documento.dump(2);
        }

        deveFalhar(
            caminho,
            "Sensor LT-01 duplicado");
    }

    // TESTE 18: nivel acima de 100%.
    {
        nlohmann::json documento = documentoValido;

        // Modifica o valor do sensor LT-01.
        documento["leituras"][0]["valor"] = 150.0;

        {
            std::ofstream arquivo(caminho);
            assert(arquivo.is_open());

            arquivo << documento.dump(2);
        }

        deveFalhar(
            caminho,
            "Valor da medicao fora dos limites");
    }

    // TESTE 19: unidade incorreta da temperatura.
    {
        nlohmann::json documento = documentoValido;

        // Temperatura deve utilizar C, nao %.
        documento["leituras"][1]["unidade"] = "%";

        {
            std::ofstream arquivo(caminho);
            assert(arquivo.is_open());

            arquivo << documento.dump(2);
        }

        deveFalhar(
            caminho,
            "Dados invalidos do sensor TT-01");
    }

    // TESTE 20: sensor desconhecido.
    {
        nlohmann::json documento = documentoValido;

        // Altera a identificacao do primeiro sensor.
        documento["leituras"][0]["tag"] = "XX-01";

        {
            std::ofstream arquivo(caminho);
            assert(arquivo.is_open());

            arquivo << documento.dump(2);
        }

        deveFalhar(
            caminho,
            "Sensor desconhecido: XX-01");
    }

    // TESTE 21: bomba com potencia negativa.
    {
        nlohmann::json documento = documentoValido;

        // Potencia abaixo do limite minimo de 0%.
        documento["atuadores"]["bomba"] = -10.0;

        {
            std::ofstream arquivo(caminho);
            assert(arquivo.is_open());

            arquivo << documento.dump(2);
        }

        deveFalhar(
            caminho,
            "Potencia de atuador fora dos limites");
    }

    // TESTE 22: aquecedor acima de 100%.
    {
        nlohmann::json documento = documentoValido;

        // Potencia acima do limite maximo de 100%.
        documento["atuadores"]["aquecedor"] = 150.0;

        {
            std::ofstream arquivo(caminho);
            assert(arquivo.is_open());

            arquivo << documento.dump(2);
        }

        deveFalhar(
            caminho,
            "Potencia de atuador fora dos limites");
    }

    // TESTE 23: bomba com valor em texto.
    {
        nlohmann::json documento = documentoValido;

        // "50" e uma string, nao um numero.
        documento["atuadores"]["bomba"] = "50";

        {
            std::ofstream arquivo(caminho);
            assert(arquivo.is_open());

            arquivo << documento.dump(2);
        }

        deveFalhar(
            caminho,
            "Potencia de atuador invalida");
    }

    // TESTE 24: aquecedor ausente.
    {
        nlohmann::json documento = documentoValido;

        // Remove o campo obrigatorio aquecedor.
        documento["atuadores"].erase("aquecedor");

        {
            std::ofstream arquivo(caminho);
            assert(arquivo.is_open());

            arquivo << documento.dump(2);
        }

        deveFalhar(
            caminho,
            "Atuador obrigatorio ausente");
    }

    // TESTE 25: ultimo_comando negativo.
    {
        nlohmann::json documento = documentoValido;

        // IDs de comandos nao podem ser negativos.
        documento["ultimo_comando"] = -1;

        {
            std::ofstream arquivo(caminho);
            assert(arquivo.is_open());

            arquivo << documento.dump(2);
        }

        deveFalhar(
            caminho,
            "Ultimo comando nao pode ser negativo");
    }

    // TESTE 26: erro_comando com tipo incorreto.
    {
        nlohmann::json documento = documentoValido;

        // O campo aceita texto ou null, nao numero.
        documento["erro_comando"] = 123;

        {
            std::ofstream arquivo(caminho);
            assert(arquivo.is_open());

            arquivo << documento.dump(2);
        }

        deveFalhar(
            caminho,
            "Campo erro_comando invalido");
    }

    // TESTE 27: motivo_seguro desconhecido.
    {
        nlohmann::json documento = documentoValido;

        // O valor nao pertence aos motivos permitidos.
        documento["motivo_seguro"] = "erro_desconhecido";

        {
            std::ofstream arquivo(caminho);
            assert(arquivo.is_open());

            arquivo << documento.dump(2);
        }

        deveFalhar(
            caminho,
            "Motivo seguro desconhecido");
    }

    // Remove o arquivo utilizado nos testes.
    std::filesystem::remove(caminho);

    std::cout
        << "27 cenarios de falha da aquisicao aprovados!\n";

    return 0;
}
