import json
import os
import subprocess
import tempfile
import unittest
from datetime import datetime, timedelta, timezone
from pathlib import Path

from planta.simulador import Planta, gravar_json


class TestIntegracaoControlador(unittest.TestCase):
    def setUp(self):
        self.dados = Path(self.enterContext(tempfile.TemporaryDirectory()))
        self.binario = os.environ["POO_CONTROLADOR"]
        self.entrada = self.dados / "planta.json"
        self.comando = self.dados / "comando_planta.json"
        self.estado = self.dados / "controlador_estado.json"
        self.telemetria = self.dados / "telemetria.json"
        self.planta = Planta(seed=7, sessao="sessao-inicial")

    def executar(self, *opcoes, erro=None):
        resultado = subprocess.run(
            [self.binario, "--dados", str(self.dados), "--uma-vez", *opcoes],
            capture_output=True,
            text=True,
            timeout=10,
        )
        if erro is None:
            self.assertEqual(resultado.returncode, 0, resultado.stderr)
        else:
            self.assertNotEqual(resultado.returncode, 0, resultado.stdout)
            self.assertIn(erro, resultado.stderr)
        return resultado

    def avancar_planta(self):
        snapshot = self.planta.passo(self.comando)
        gravar_json(self.entrada, snapshot)
        return snapshot

    def ler(self, caminho):
        return json.loads(caminho.read_text(encoding="utf-8"))

    def saidas(self):
        return tuple(caminho.read_bytes() for caminho in
                     (self.comando, self.estado, self.telemetria))

    def test_selecao_das_politicas(self):
        snapshot = self.avancar_planta()
        self.executar()
        self.assertEqual(self.ler(self.comando)["atuadores"],
                         {"bomba": 100, "aquecedor": 100})

        # Uma execução independente avalia exatamente o mesmo snapshot.
        for caminho in (self.comando, self.estado, self.telemetria):
            caminho.unlink()
        self.executar("--politica", "faixas")
        self.assertEqual(self.ler(self.comando)["atuadores"],
                         {"bomba": 70, "aquecedor": 70})
        self.assertEqual(self.ler(self.telemetria)["atuadores"], snapshot["atuadores"])

    def test_politica_invalida_ou_ausente(self):
        self.avancar_planta()
        for opcoes, erro in [
            (("--politica", "desconhecida"), "Politica desconhecida"),
            (("--politica",), "Informe on-off ou faixas"),
        ]:
            with self.subTest(opcoes=opcoes):
                self.executar(*opcoes, erro=erro)
                for caminho in (self.comando, self.estado, self.telemetria):
                    self.assertFalse(caminho.exists(), caminho)

    def test_confirmacao_repeticao_regressao_e_nova_sessao(self):
        primeiro_snapshot = self.avancar_planta()
        self.executar()
        primeiro_pedido = self.ler(self.comando)
        self.assertEqual(primeiro_pedido["id"], 1)
        segundo_snapshot = self.avancar_planta()
        self.assertEqual(segundo_snapshot["ultimo_comando"], primeiro_pedido["id"])
        self.assertEqual(segundo_snapshot["atuadores"], primeiro_pedido["atuadores"])

        self.executar()
        self.assertEqual(self.ler(self.estado)["ultimo_comando_confirmado"], 1)
        self.assertEqual(self.ler(self.comando)["id"], 2)
        self.assertEqual(self.ler(self.telemetria)["ultimo_comando"], 1)
        anteriores = self.saidas()
        self.executar()
        self.assertEqual(self.saidas(), anteriores)
        gravar_json(self.entrada, primeiro_snapshot)
        self.executar(erro="Ciclo antigo")
        self.assertEqual(self.saidas(), anteriores)

        self.planta = Planta(seed=7, sessao="sessao-reiniciada")
        self.assertEqual(self.avancar_planta()["ultimo_comando"], 0)
        self.executar()
        novo_pedido = self.ler(self.comando)
        self.assertEqual((novo_pedido["sessao"], novo_pedido["id"]), ("sessao-reiniciada", 1))
        self.assertEqual(self.ler(self.estado)["ultimo_comando_confirmado"], 0)
        self.assertEqual(self.avancar_planta()["ultimo_comando"], 1)

    def test_falha_de_publicacao_e_recuperacao_do_pedido(self):
        self.avancar_planta()
        temporario = self.comando.with_suffix(".json.tmp")
        temporario.mkdir()
        self.executar(erro="Nao foi possivel abrir o comando temporario")
        self.assertFalse(self.comando.exists())
        reserva = self.ler(self.estado)
        self.assertEqual(reserva["ciclo"], 0)
        self.assertEqual(reserva["ultimo_comando_confirmado"], 0)
        self.assertEqual(reserva["solicitacao"], {
            "ciclo": 1, "id": 1, "bomba": 100, "aquecedor": 100,
        })
        temporario.rmdir()

        esperado = {
            "versao": 1, "sessao": reserva["sessao"], "id": 1,
            "atuadores": {"bomba": 100, "aquecedor": 100},
        }
        for conteudo in (
            None,
            "{",
            json.dumps({"id": 1}),
            json.dumps({**esperado, "versao": 1.0}),
            json.dumps({**esperado, "id": True}),
            json.dumps({**esperado, "atuadores": {"bomba": 50, "aquecedor": 100}}),
            json.dumps(esperado),
        ):
            with self.subTest(conteudo=conteudo):
                gravar_json(self.estado, reserva)
                self.comando.unlink(missing_ok=True)
                if conteudo is not None:
                    self.comando.write_text(conteudo, encoding="utf-8")
                self.executar("--politica", "faixas")
                self.assertEqual(self.ler(self.comando), esperado)
                self.assertIs(type(self.ler(self.comando)["versao"]), int)
                self.assertIs(type(self.ler(self.comando)["id"]), int)
                if conteudo == json.dumps(esperado):
                    self.assertEqual(self.comando.read_text(encoding="utf-8"), conteudo)
                self.assertEqual(self.ler(self.estado)["ciclo"], 1)
                self.assertEqual(self.ler(self.estado)["ultimo_comando_confirmado"], 0)

        self.assertEqual(self.avancar_planta()["ultimo_comando"], 1)

    def test_estado_invalido_preserva_saidas(self):
        self.avancar_planta()
        self.executar()
        self.avancar_planta()
        self.estado.write_text("{", encoding="utf-8")
        anteriores = self.saidas()
        self.executar(erro="Arquivo de estado JSON invalido")
        self.assertEqual(self.saidas(), anteriores)

    def test_falha_de_escrita_da_telemetria_impede_novo_comando(self):
        self.avancar_planta()
        self.executar()
        self.avancar_planta()
        anteriores = self.saidas()
        self.telemetria.with_suffix(".json.tmp").mkdir()
        self.executar(erro="Nao foi possivel criar a telemetria temporaria")
        self.assertEqual(self.saidas(), anteriores)
        self.executar()
        self.assertEqual(self.ler(self.comando)["id"], 2)

    def test_timestamps_invalidos_preservam_saidas(self):
        self.avancar_planta()
        self.executar()
        snapshot = self.avancar_planta()
        anteriores = self.saidas()
        agora = datetime.now(timezone.utc)
        for dias, erro in [(-1, "Aquisicao expirada"), (1, "Timestamp no futuro")]:
            with self.subTest(dias=dias):
                snapshot["timestamp"] = (agora + timedelta(days=dias)).strftime("%Y-%m-%dT%H:%M:%SZ")
                gravar_json(self.entrada, snapshot)
                self.executar(erro=erro)
                self.assertEqual(self.saidas(), anteriores)

    def test_alarmes_e_falha_de_sensor(self):
        snapshot = self.avancar_planta()
        snapshot["leituras"][0]["valor"] = 4.5
        snapshot["leituras"][1]["valor"] = 41.0
        gravar_json(self.entrada, snapshot)
        self.executar()
        self.assertEqual(
            [(alarme["codigo"], alarme["severidade"])
             for alarme in self.ler(self.telemetria)["alarmes"]],
            [("NIVEL_CRITICO", "critico"), ("TEMP_ALTA", "aviso")],
        )
        pedido = self.comando.read_bytes()
        for indice_falha, valor_saudavel, codigo in [
            (0, 18.0, "TEMP_BAIXA"),
            (1, 95.0, "NIVEL_ALTO"),
        ]:
            with self.subTest(sensor=indice_falha):
                snapshot = self.avancar_planta()
                snapshot["leituras"][indice_falha].update(valor=None, status="falha")
                snapshot["leituras"][1 - indice_falha]["valor"] = valor_saudavel
                gravar_json(self.entrada, snapshot)
                self.executar()
                telemetria = self.ler(self.telemetria)
                self.assertEqual(telemetria["leituras"], snapshot["leituras"])
                self.assertEqual(telemetria["atuadores"], snapshot["atuadores"])
                self.assertEqual([a["codigo"] for a in telemetria["alarmes"]], [codigo])
                self.assertEqual(self.comando.read_bytes(), pedido)
                self.assertEqual(self.ler(self.estado)["ciclo"], snapshot["ciclo"])
