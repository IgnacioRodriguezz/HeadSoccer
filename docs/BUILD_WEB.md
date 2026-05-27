# Compilar HeadSoccer para web (Emscripten)

## 1. Instalar emsdk

    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh        # Linux/Mac
    # Windows: emsdk_env.bat

## 2. Compilar Raylib para web

    git clone https://github.com/raysan5/raylib.git
    cd raylib/src
    emmake make PLATFORM=PLATFORM_WEB -B
    # Genera: raylib/src/libraylib.a

## 3. Compilar HeadSoccer

    cd HeadSoccer/
    emmake make -f Makefile.web RAYLIB_LIB=../raylib/src/libraylib.a RAYLIB_INC=../raylib/src

## 4. Probar localmente

    python -m http.server 8000
    # Abrir: http://localhost:8000/web/headsoccer.html
    # (No abrir el .html directo — WASM requiere servidor HTTP)
