# =============================================================================
# Makefile — N-body Simulator
#
# Alvos principais:
#   make          → compila o simulador principal
#   make test     → compila e roda os testes unitários
#   make valgrind → roda simulação com verificação de memória
#   make clean    → remove arquivos gerados
#   make help     → exibe esta ajuda
# =============================================================================

# Compilador e flags
CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -g -O2
LDFLAGS = -lm

# Diretórios
SRC_DIR   = src
TEST_DIR  = tests
DATA_DIR  = data
BUILD_DIR = build

# Arquivos fonte do simulador (sem main.c para os testes)
SOURCES = $(SRC_DIR)/body.c     \
          $(SRC_DIR)/physics.c  \
          $(SRC_DIR)/integrator.c \
          $(SRC_DIR)/io.c       \
          $(SRC_DIR)/renderer.c \
          $(SRC_DIR)/cJSON.c

MAIN_SRC = $(SRC_DIR)/main.c

# Unity framework (single C file)
UNITY_SRC = $(TEST_DIR)/unity.c

# Arquivos objeto
OBJECTS      = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))
MAIN_OBJ     = $(BUILD_DIR)/main.o
UNITY_OBJ    = $(BUILD_DIR)/unity.o

# Executáveis
TARGET       = nbody
TEST_PHYSICS = $(BUILD_DIR)/test_physics
TEST_INTEGR  = $(BUILD_DIR)/test_integrator

# Cria diretório de build se não existir
$(shell mkdir -p $(BUILD_DIR) $(DATA_DIR))

# =============================================================================
# Alvo padrão: compila o simulador principal
# =============================================================================
all: $(TARGET)

$(TARGET): $(OBJECTS) $(MAIN_OBJ)
	@echo "[LINK] $@"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "[OK]   Compilado: $(TARGET)"

# Regra genérica: compila cada .c em .o no diretório build
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "[CC]   $<"
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	@echo "[CC]   $<"
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

# =============================================================================
# Testes unitários com Unity
# =============================================================================
test: $(TEST_PHYSICS) $(TEST_INTEGR)
	@echo ""
	@echo "=== Rodando: test_physics ==="
	./$(TEST_PHYSICS)
	@echo ""
	@echo "=== Rodando: test_integrator ==="
	./$(TEST_INTEGR)
	@echo ""
	@echo "[OK]   Todos os testes concluídos."

$(UNITY_OBJ): $(TEST_DIR)/unity.c
	@echo "[CC]   unity.c"
	$(CC) $(CFLAGS) -I$(TEST_DIR) -c $< -o $@

$(TEST_PHYSICS): $(TEST_DIR)/test_physics.c $(OBJECTS) $(UNITY_OBJ)
	@echo "[LINK] test_physics"
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(TEST_DIR) \
	      $< $(OBJECTS) $(UNITY_OBJ) -o $@ $(LDFLAGS)

$(TEST_INTEGR): $(TEST_DIR)/test_integrator.c $(OBJECTS) $(UNITY_OBJ)
	@echo "[LINK] test_integrator"
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(TEST_DIR) \
	      $< $(OBJECTS) $(UNITY_OBJ) -o $@ $(LDFLAGS)

# =============================================================================
# Verificação de vazamento de memória com Valgrind
# =============================================================================
valgrind: $(TARGET)
	@echo "[RUN]  valgrind --leak-check=full ./$(TARGET) config/solar_system.json --no-plot"
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
	         --error-exitcode=1 \
	         ./$(TARGET) config/solar_system.json --no-plot

# =============================================================================
# Limpeza
# =============================================================================
clean:
	@echo "[RM]   $(BUILD_DIR)/ $(TARGET)"
	rm -rf $(BUILD_DIR) $(TARGET) $(DATA_DIR)/output.dat
	@echo "[OK]   Limpeza concluída."

# =============================================================================
# Ajuda
# =============================================================================
help:
	@echo "N-Gravity Simulator — Makefile"
	@echo ""
	@echo "Alvos disponíveis:"
	@echo "  make           → compila o simulador (./nbody)"
	@echo "  make test      → compila e roda testes unitários (Unity)"
	@echo "  make valgrind  → roda com verificação de memória"
	@echo "  make clean     → remove build/ e binários"
	@echo "  make help      → esta mensagem"
	@echo ""
	@echo "Uso:"
	@echo "  ./nbody config/solar_system.json"
	@echo "  ./nbody config/three_body_chaos.json --no-plot"

.PHONY: all test valgrind clean help
