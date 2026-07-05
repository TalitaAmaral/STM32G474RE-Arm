import struct
import json
import sys

MAGIC = 0x54524331
HDR_SIZE = 16
EVT_SIZE = 8

# DICIONÁRIO DE NOMES: Vincula o ID numérico da tarefa ao nome real do código
nomes = {
    0: "Idle",
    1: "Blinky1",
    2: "Blinky2",
    3: "Blinky3",
    4: "Produtor",
    5: "Consumidor"
}

def converter_para_perfetto(bin_path, json_path):
    data = open(bin_path, "rb").read()
    magic, clock, count, capacity = struct.unpack_from("<IIII", data, 0)
    
    if magic != MAGIC:
        print("Erro: Arquivo binário inválido!")
        return

    # Pula o cabeçalho original de metadados
    off = HDR_SIZE + (8 * 12) 

    count = min(count, capacity, (len(data) - off) // EVT_SIZE)
    
    perfetto_events = []
    
    # Rastreia o estado de execução de cada Thread ID (tid)
    tarefa_atual = {} 

    for i in range(count):
        cyc, code, idx, arg = struct.unpack_from("<IBBH", data, off + i * EVT_SIZE)
        t_us = (cyc * 1000000.0) / clock 
        
        nome_tarefa = nomes.get(idx, f"Thread_{idx}")
        tid_alvo = int(idx)

        if code == 2: # TR_RUN (A tarefa ganhou a CPU)
            # Se já havia um bloco aberto neste mesmo canal, fecha antes de abrir o próximo
            if tarefa_atual.get(tid_alvo):
                perfetto_events.append({
                    "name": nome_tarefa, "ph": "E", "ts": t_us, "pid": 1, "tid": tid_alvo
                })
            
            # Abre o novo bloco de execução
            perfetto_events.append({
                "name": nome_tarefa, "ph": "B", "ts": t_us, "pid": 1, "tid": tid_alvo
            })
            tarefa_atual[tid_alvo] = True
                
        elif code == 4: # TR_BLOCK (A tarefa perdeu a CPU / Dormiu)
            # Se havia um bloco aberto para ela, fecha imediatamente
            if tarefa_atual.get(tid_alvo):
                perfetto_events.append({
                    "name": nome_tarefa, "ph": "E", "ts": t_us, "pid": 1, "tid": tid_alvo
                })
                tarefa_atual[tid_alvo] = False

    # Fecha qualquer bloco que tenha restado aberto no fim do arquivo de log
    for tid_alvo, ativo in tarefa_atual.items():
        if ativo:
            nome_tarefa = nomes.get(tid_alvo, f"Thread_{tid_alvo}")
            perfetto_events.append({
                "name": nome_tarefa, "ph": "E", "ts": t_us, "pid": 1, "tid": tid_alvo
            })

    with open(json_path, "w") as f:
        json.dump(perfetto_events, f, indent=2)
    print(f"Arquivo gerado em: {json_path}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python converter_perfetto.py trace")
    else:
        converter_para_perfetto(sys.argv[1], "trace_perfetto.json")
