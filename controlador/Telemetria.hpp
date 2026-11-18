
#pragma once

#include "Aquisicao.hpp"

#include <filesystem>

class Telemetria {

public:

    static void publicar(
        const DadosPlanta& dados,
        const std::filesystem::path& caminho);
};
