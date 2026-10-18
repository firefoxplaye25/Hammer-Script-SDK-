# INSTRUÇÕES CRÍTICAS DE ARQUITETURA E DISTRIBUIÇÃO PÚBLICA (LEIA ANTES DE COMPILAR)

Este documento contém os requisitos obrigatórios de organização de pastas, empacotamento e a especificação do subsistema **Lib Loader** antes de gerar os binários públicos de distribuição do compilador/SDK do Hammer Script.

---

## 1. Organização Estruturada da Pasta de Distribuição (`dist/` ou `release/`)

Ao compilar a versão pública, a pasta raiz final distribuível DEVE seguir rigorosamente esta árvore:

```text
hammer-sdk/
├── bin/
│   ├── hammer.exe              # CLI principal do ecossistema
│   ├── hamc.exe                # Transpiler C++ nativo
│   └── hamhook.exe             # Primitive EXE compiler / linker
├── lib loader/                 # Pasta dedicada para módulos externos e plugins
│   ├── WARNING_SECURITY.txt    # Aviso de segurança obrigatório (ver seção 2)
│   └── community/              # Diretório padrão de instalação de libs de terceiros
├── hammer library header/      # Manifestos das bibliotecas nativas (.lib)
│   ├── asock.lib
│   ├── hamfs.lib
│   ├── hmhook.lib
│   ├── hmhookwww.lib
│   └── hmripa.lib
├── stdlib/                     # Módulos padrão em Hammer Script (.hmscp)
├── docs/                       # Documentação da sintaxe e exemplos
├── LICENSE
└── README.md








//////////// compile na pasta "Build" se for pra publico/////////