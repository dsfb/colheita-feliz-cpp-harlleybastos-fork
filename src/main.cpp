// FILE: src/main.cpp

// Inclui o header principal do SDL2 — dá acesso a funções de janela,
// renderização, eventos de input, e controle de tempo
#include <SDL2/SDL.h>

// Inclui o header do SDL2_image — vamos precisar dele inicializado
// mesmo que ainda não carreguemos imagens nesse step, porque no próximo
// step já vamos usar e é boa prática inicializar tudo junto
#include <SDL2/SDL_image.h>

// Inclui o header do SDL2_mixer — sistema de áudio, inicializado
// junto com tudo pra não ter que refatorar depois
#include <SDL2/SDL_mixer.h>

// Inclui o header do SDL2_ttf — renderização de texto,
// também inicializado desde já
#include <SDL2/SDL_ttf.h>

// Inclui o nlohmann_json — biblioteca pra JSON que vamos usar no save/load
// Incluída aqui pra manter o build testando todas as dependências
#include <nlohmann/json.hpp>

// Inclui iostream pra mensagens de debug no console
// std::cout e std::cerr são essenciais durante o desenvolvimento
#include <iostream>

// NOVO: Largura da janela do jogo em pixels
// 1280 é uma resolução 16:9 comum que funciona bem na maioria dos monitores
// e dá espaço suficiente pro mapa isométrico + UI
constexpr int LARGURA_JANELA = 1280;

// NOVO: Altura da janela do jogo em pixels
// 720 complementa os 1280 pra formar 720p (HD), uma resolução segura
constexpr int ALTURA_JANELA = 720;

// NOVO: Taxa de frames por segundo que queremos manter
// 60 FPS é o padrão da indústria — suave o suficiente pro olho humano,
// leve o suficiente pra rodar em qualquer PC moderno
constexpr int FPS_ALVO = 60;

// NOVO: Tempo mínimo que cada frame deve durar, em milissegundos
// 1000ms / 60 FPS = ~16.6ms por frame
// Usamos isso pra limitar a velocidade do loop e não fritar a CPU
constexpr int TEMPO_FRAME_MS = 1000 / FPS_ALVO;

// Ponto de entrada do programa
// argc e argv são argumentos de linha de comando — SDL2 exige essa assinatura
// por causa do SDL2main que redireciona o ponto de entrada no Windows
int main(int argc, char* argv[]) {

    // --- INICIALIZAÇÃO ---

    // SDL_Init() inicializa os subsistemas do SDL2
    // SDL_INIT_VIDEO ativa janela e renderização
    // SDL_INIT_AUDIO ativa o sistema de áudio
    // O operador | combina as duas flags (bitwise OR)
    // Retorna 0 se sucesso, negativo se erro
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        // SDL_GetError() retorna uma string com a descrição do último erro
        std::cerr << "Erro SDL_Init: " << SDL_GetError() << std::endl;
        return 1; // Encerra o programa com código de erro
    }

    // IMG_Init() inicializa o SDL2_image com suporte a PNG
    // IMG_INIT_PNG ativa o decoder de imagens PNG (que têm transparência)
    // Retorna as flags que foram inicializadas — checamos com & (bitwise AND)
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "Erro IMG_Init: " << IMG_GetError() << std::endl;
        SDL_Quit(); // Limpa o SDL2 antes de sair
        return 1;
    }

    // TTF_Init() inicializa o SDL2_ttf pra renderização de texto com fontes TrueType
    // Retorna 0 se sucesso, -1 se erro
    if (TTF_Init() == -1) {
        std::cerr << "Erro TTF_Init: " << TTF_GetError() << std::endl;
        IMG_Quit(); // Limpa SDL2_image
        SDL_Quit(); // Limpa SDL2
        return 1;
    }

    // Mix_OpenAudio() abre o dispositivo de áudio com as configurações especificadas
    // 44100 = frequência em Hz (qualidade de CD)
    // MIX_DEFAULT_FORMAT = formato de áudio padrão da plataforma (16-bit)
    // 2 = canais de áudio (estéreo)
    // 2048 = tamanho do buffer em bytes (equilíbrio entre latência e performance)
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Erro Mix_OpenAudio: " << Mix_GetError() << std::endl;
        TTF_Quit();  // Limpa SDL2_ttf
        IMG_Quit();  // Limpa SDL2_image
        SDL_Quit();  // Limpa SDL2
        return 1;
    }

    // NOVO: SDL_CreateWindow() cria a janela do jogo
    // "Colheita Feliz" = título que aparece na barra de título da janela
    // SDL_WINDOWPOS_CENTERED = posiciona a janela no centro da tela (horizontal)
    // SDL_WINDOWPOS_CENTERED = posiciona a janela no centro da tela (vertical)
    // LARGURA_JANELA, ALTURA_JANELA = dimensões da janela (1280x720)
    // SDL_WINDOW_SHOWN = a janela aparece imediatamente ao ser criada
    // Retorna um ponteiro pra janela, ou nullptr se falhou
    SDL_Window* janela = SDL_CreateWindow(
        "Colheita Feliz",          // Título da janela
        SDL_WINDOWPOS_CENTERED,    // Posição X (centralizada)
        SDL_WINDOWPOS_CENTERED,    // Posição Y (centralizada)
        LARGURA_JANELA,            // Largura em pixels
        ALTURA_JANELA,             // Altura em pixels
        SDL_WINDOW_SHOWN           // Flags — mostra a janela ao criar
    );

    // Verifica se a janela foi criada com sucesso
    if (!janela) {
        std::cerr << "Erro ao criar janela: " << SDL_GetError() << std::endl;
        Mix_CloseAudio(); // Fecha áudio
        TTF_Quit();       // Limpa SDL2_ttf
        IMG_Quit();       // Limpa SDL2_image
        SDL_Quit();       // Limpa SDL2
        return 1;
    }

    // NOVO: SDL_CreateRenderer() cria o renderer — o "pincel" que desenha na janela
    // O renderer é o objeto que efetivamente coloca pixels na tela
    // janela = a janela onde ele vai desenhar
    // -1 = usa o primeiro driver de renderização disponível (geralmente aceleração por GPU)
    // SDL_RENDERER_ACCELERATED = usa a GPU pra renderizar (muito mais rápido que CPU)
    // SDL_RENDERER_PRESENTVSYNC = sincroniza com o monitor pra evitar "screen tearing"
    //   (aquele efeito de imagem cortada ao meio quando o jogo desenha mais rápido que o monitor atualiza)
    SDL_Renderer* renderer = SDL_CreateRenderer(
        janela,                                            // A janela alvo
        -1,                                                // Índice do driver (-1 = automático)
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC  // Flags de renderização
    );

    // Verifica se o renderer foi criado com sucesso
    if (!renderer) {
        std::cerr << "Erro ao criar renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(janela); // Destrói a janela antes de sair
        Mix_CloseAudio();          // Fecha áudio
        TTF_Quit();                // Limpa SDL2_ttf
        IMG_Quit();                // Limpa SDL2_image
        SDL_Quit();                // Limpa SDL2
        return 1;
    }

    std::cout << "Janela e renderer criados com sucesso!" << std::endl;

    // --- GAME LOOP ---

    // NOVO: Flag que controla se o jogo está rodando
    // Quando o jogador fechar a janela (clicar no X), mudamos pra false e o loop para
    bool rodando = true;

    // NOVO: SDL_Event é uma struct que guarda informações sobre eventos
    // Eventos são coisas que acontecem: tecla pressionada, mouse mexeu, janela fechada, etc.
    // Declaramos fora do loop pra não ficar criando e destruindo a cada frame
    SDL_Event evento;

    // NOVO: Variáveis pra controle de tempo (delta time)
    // SDL_GetTicks() retorna quantos milissegundos se passaram desde que o SDL2 foi inicializado
    Uint32 tempo_anterior = SDL_GetTicks(); // Marca o tempo no início do loop

    // NOVO: deltaTime é o tempo em SEGUNDOS que passou entre o frame anterior e o atual
    // Vamos usar isso pra fazer movimentos e animações independentes de FPS
    float deltaTime = 0.0f;

    // NOVO: O game loop — roda enquanto "rodando" for true
    // Cada volta desse while = um frame do jogo
    while (rodando) {

        // NOVO: Calcula o delta time
        // tempo_atual = milissegundos desde o início do programa
        Uint32 tempo_atual = SDL_GetTicks();
        // Subtrai o tempo anterior pra saber quanto tempo passou neste frame
        // Divide por 1000.0f pra converter de milissegundos pra segundos
        // Ex: se passaram 16ms, deltaTime = 0.016 segundos
        deltaTime = (tempo_atual - tempo_anterior) / 1000.0f;
        // Atualiza o tempo anterior pro próximo frame
        tempo_anterior = tempo_atual;

        // --- 1. CAPTURA DE EVENTOS (INPUT) ---

        // NOVO: SDL_PollEvent() verifica se há eventos na fila
        // Retorna 1 se pegou um evento, 0 se a fila está vazia
        // O while processa TODOS os eventos que aconteceram desde o último frame
        // (pode ter vários: tecla pressionada + mouse mexeu, por exemplo)
        while (SDL_PollEvent(&evento)) {

            // NOVO: evento.type diz qual tipo de evento aconteceu
            // SDL_QUIT é emitido quando o jogador clica no X da janela
            if (evento.type == SDL_QUIT) {
                rodando = false; // Sai do game loop
            }

            // NOVO: SDL_KEYDOWN é emitido quando uma tecla é pressionada
            if (evento.type == SDL_KEYDOWN) {
                // evento.key.keysym.sym contém qual tecla foi pressionada
                // SDLK_ESCAPE = tecla Esc
                if (evento.key.keysym.sym == SDLK_ESCAPE) {
                    rodando = false; // Esc também fecha o jogo (atalho útil durante dev)
                }
            }
        }

        // --- 2. ATUALIZAÇÃO DA LÓGICA ---

        // Por enquanto não temos lógica de jogo ainda
        // Aqui é onde vamos atualizar posição do jogador, crescimento das plantas,
        // movimento dos animais, etc. — tudo usando deltaTime pra ser independente de FPS
        // Exemplo futuro: jogador.x += velocidade * deltaTime;

        // --- 3. RENDERIZAÇÃO ---

        // NOVO: SDL_SetRenderDrawColor() define a cor que o renderer vai usar
        // Os parâmetros são R, G, B, A (vermelho, verde, azul, transparência)
        // Cada valor vai de 0 a 255
        // (34, 139, 34, 255) = um verde floresta — lembra grama de fazenda
        // 255 no alpha = totalmente opaco
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);

        // NOVO: SDL_RenderClear() preenche TODA a tela com a cor definida acima
        // Isso "limpa" o frame anterior — sem isso, os frames se sobrepõem e vira uma bagunça
        SDL_RenderClear(renderer);

        // Aqui é onde vamos desenhar tudo: mapa, personagem, UI, etc.
        // Por enquanto, só a tela verde — mas já é o nosso "canvas" pronto

        // NOVO: SDL_RenderPresent() mostra na tela tudo que foi desenhado
        // O SDL2 usa "double buffering": enquanto um frame aparece na tela,
        // o próximo está sendo desenhado em memória. RenderPresent() troca os dois
        // Isso evita aquele efeito de tela piscando (flickering)
        SDL_RenderPresent(renderer);

        // --- CONTROLE DE FPS ---

        // NOVO: Calcula quanto tempo esse frame levou pra processar
        Uint32 tempo_frame = SDL_GetTicks() - tempo_atual;

        // NOVO: Se o frame foi processado rápido demais, espera o tempo restante
        // Isso evita que o loop rode a 1000+ FPS e frite a CPU à toa
        // Exemplo: se o frame levou 4ms e o alvo é 16ms, espera 12ms
        if (tempo_frame < TEMPO_FRAME_MS) {
            // SDL_Delay() pausa o programa pelo número de milissegundos especificado
            // Libera a CPU pra fazer outras coisas enquanto espera
            SDL_Delay(TEMPO_FRAME_MS - tempo_frame);
        }
    }

    // --- LIMPEZA (SHUTDOWN) ---

    // NOVO: Destrói o renderer — libera os recursos de GPU que ele usava
    SDL_DestroyRenderer(renderer);

    // NOVO: Destrói a janela — fecha ela e libera a memória
    SDL_DestroyWindow(janela);

    // Finaliza todas as bibliotecas na ordem inversa de inicialização
    // Ordem inversa evita problemas de dependência (ex: mixer depende do SDL2,
    // então fechamos o mixer antes do SDL2)
    Mix_CloseAudio(); // Fecha o sistema de áudio
    TTF_Quit();       // Finaliza o SDL2_ttf
    IMG_Quit();       // Finaliza o SDL2_image
    SDL_Quit();       // Finaliza o SDL2 — sempre por último

    std::cout << "Jogo encerrado com sucesso!" << std::endl;
    return 0; // Retorna 0 indicando que tudo correu bem
}
