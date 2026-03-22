// FILE: src/main.cpp

// Inclui o header principal do SDL2 — isso dá acesso a todas as funções
// básicas como criar janela, capturar eventos, desenhar na tela, etc.
// O <SDL2/SDL.h> funciona porque o pacman instalou os headers em /ucrt64/include/SDL2/
#include <SDL2/SDL.h>

// Inclui o header do SDL2_image — funções pra carregar PNG, JPG, etc.
#include <SDL2/SDL_image.h>

// Inclui o header do SDL2_mixer — funções pra tocar sons e música
#include <SDL2/SDL_mixer.h>

// Inclui o header do SDL2_ttf — funções pra carregar fontes e renderizar texto
#include <SDL2/SDL_ttf.h>

// Inclui o header do nlohmann_json — pra trabalhar com arquivos JSON (save/load)
#include <nlohmann/json.hpp>

// Inclui iostream pra poder usar std::cout — exibir mensagens no console
// Útil pra debug durante o desenvolvimento
#include <iostream>

// Ponto de entrada do programa
// argc e argv são argumentos de linha de comando — não vamos usar, mas o SDL2
// exige essa assinatura exata no main (por causa do SDL2main que faz umas mágicas internas)
int main(int argc, char* argv[]) {

    // SDL_Init() inicializa os subsistemas do SDL2
    // SDL_INIT_VIDEO ativa a parte de vídeo (janela, renderização)
    // SDL_INIT_AUDIO ativa a parte de áudio (pra usar o SDL2_mixer depois)
    // O | (pipe/ou) combina as duas flags numa só chamada
    // Retorna 0 se deu certo, número negativo se falhou
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        // SDL_GetError() retorna uma string descrevendo o último erro do SDL2
        // Útil demais pra debug — sempre use quando uma função SDL falhar
        std::cout << "Erro ao inicializar SDL2: " << SDL_GetError() << std::endl;
        return 1; // Retorna 1 pro sistema operacional indicando que deu erro
    }

    // Mensagem de sucesso — se aparecer no console, SDL2 está funcionando
    std::cout << "SDL2 inicializado com sucesso!" << std::endl;
    // Mostra a versão pra confirmar que é a versão que instalamos
    SDL_version versao; // Struct que guarda major.minor.patch
    SDL_GetVersion(&versao); // Preenche a struct com a versão atual
    std::cout << "Versao SDL2: " << (int)versao.major << "." << (int)versao.minor << "." << (int)versao.patch << std::endl;

    // IMG_Init() inicializa o SDL2_image com suporte a PNG
    // IMG_INIT_PNG é uma flag que ativa o decoder de PNG
    // Retorna as flags que foram inicializadas com sucesso
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cout << "Erro ao inicializar SDL2_image: " << IMG_GetError() << std::endl;
        SDL_Quit(); // Limpa o SDL2 antes de sair
        return 1;
    }
    std::cout << "SDL2_image (PNG) inicializado com sucesso!" << std::endl;

    // TTF_Init() inicializa o SDL2_ttf pra renderização de texto
    // Retorna 0 se deu certo, -1 se falhou
    if (TTF_Init() == -1) {
        std::cout << "Erro ao inicializar SDL2_ttf: " << TTF_GetError() << std::endl;
        IMG_Quit(); // Limpa o SDL2_image
        SDL_Quit(); // Limpa o SDL2
        return 1;
    }
    std::cout << "SDL2_ttf inicializado com sucesso!" << std::endl;

    // Mix_OpenAudio() inicializa o sistema de áudio do SDL2_mixer
    // 44100 = frequência de áudio em Hz (qualidade de CD, padrão da indústria)
    // MIX_DEFAULT_FORMAT = formato de áudio padrão da plataforma (16-bit)
    // 2 = número de canais (2 = estéreo, som nos dois lados do fone)
    // 2048 = tamanho do buffer de áudio em bytes — 2048 é um bom equilíbrio
    //         entre latência baixa e performance (buffer menor = resposta mais rápida,
    //         mas pode causar chiados se o PC for lento)
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "Erro ao inicializar SDL2_mixer: " << Mix_GetError() << std::endl;
        TTF_Quit();  // Limpa o SDL2_ttf
        IMG_Quit();  // Limpa o SDL2_image
        SDL_Quit();  // Limpa o SDL2
        return 1;
    }
    std::cout << "SDL2_mixer inicializado com sucesso!" << std::endl;

    // Testa o nlohmann_json criando um objeto JSON simples
    // nlohmann::json funciona como um "dicionário" — você coloca chave-valor
    // e depois pode converter pra string, salvar em arquivo, etc.
    nlohmann::json teste; // Cria um objeto JSON vazio
    teste["jogo"] = "Colheita Feliz"; // Adiciona um campo de texto
    teste["versao"] = 1;              // Adiciona um campo numérico
    // .dump(2) converte o JSON pra string formatada com indentação de 2 espaços
    std::cout << "nlohmann_json funcionando! Teste: " << teste.dump(2) << std::endl;

    // Agora limpamos tudo na ordem inversa de inicialização
    // Isso é uma boa prática em C++ — liberar recursos na ordem inversa
    // evita dependências "penduradas" (ex: fechar o SDL antes do mixer que depende dele)
    Mix_CloseAudio(); // Fecha o sistema de áudio
    TTF_Quit();       // Finaliza o SDL2_ttf
    IMG_Quit();       // Finaliza o SDL2_image
    SDL_Quit();       // Finaliza o SDL2 (sempre por último)

    std::cout << "\nTudo funcionando! Ambiente pronto para o Colheita Feliz!" << std::endl;
    return 0; // Retorna 0 pro sistema operacional indicando que deu tudo certo
}
