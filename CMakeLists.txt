cmake_minimum_required(VERSION 3.16)

project(CompiladorIDE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Requer Qt 6 (qt_add_resources com FILES)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Arquivos gerados pelo GALS (não editar à mão; regenerar pelo Web-GALS)
set(GALS_SOURCES
    Constants.cpp Constants.h
    Lexico.cpp Lexico.h
    Sintatico.cpp Sintatico.h
    Token.h
    AnalysisError.h LexicalError.h SyntacticError.h SemanticError.h
)

# Analisador semântico (escrito à mão; o GALS gera um Semantico.* vazio, não copie por cima)
set(SEMANTICO_SOURCES
    Semantico.cpp Semantico.h
    TabelaSimbolos.cpp TabelaSimbolos.h
    Tipos.h SemanticTable.h
    GeradorBIP.cpp GeradorBIP.h
)

# Catálogo de todos os tokens da linguagem (aba Tokens; conferido pelo rodar_casos)
set(TOKENS_SOURCES CatalogoTokens.cpp CatalogoTokens.h)

# Arquivos da IDE
set(IDE_SOURCES
    main.cpp
    MainWindow.cpp MainWindow.h
    Compilador.cpp Compilador.h
    Estandarte.cpp Estandarte.h
)

add_executable(CompiladorIDE ${IDE_SOURCES} ${GALS_SOURCES} ${SEMANTICO_SOURCES} ${TOKENS_SOURCES})

target_link_libraries(CompiladorIDE PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Fontes (todas com licença livre: SIL OFL) e imagens embutidas no executável
qt_add_resources(CompiladorIDE "recursos"
    PREFIX "/"
    FILES
        fontes/Cinzel-VariableFont_wght.ttf
        fontes/Cinzel-Black.ttf
        fontes/MarcellusSC-Regular.ttf
        fontes/Almendra-Italic.ttf
        fontes/Alegreya-VariableFont_wght.ttf
        fontes/Almendra-Regular.ttf
        imagens/papiro.jpg
        imagens/marmore.jpg
        imagens/tecido.jpg
)

if(MSVC)
    target_compile_options(CompiladorIDE PRIVATE /utf-8)
endif()

set_target_properties(CompiladorIDE PROPERTIES WIN32_EXECUTABLE ON)

# Copia as DLLs do Qt e do runtime do compilador para junto do .exe,
# permitindo abri-lo direto pelo Explorer (fora do Qt Creator)
if(WIN32)
    get_target_property(QMAKE_EXE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)
    find_program(WINDEPLOYQT windeployqt HINTS "${QT_BIN_DIR}")

    get_filename_component(COMPILER_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

    if(WINDEPLOYQT)
        add_custom_command(TARGET CompiladorIDE POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E env "PATH=${COMPILER_BIN_DIR};${QT_BIN_DIR}"
                    "${WINDEPLOYQT}" --no-translations --compiler-runtime
                    "$<TARGET_FILE:CompiladorIDE>"
            COMMENT "Copiando DLLs do Qt (windeployqt)..."
        )
    endif()
endif()

# Testes automáticos, sem Qt: roda todos os casos de casos_de_teste.txt
#   cmake --build build --target testar
add_executable(rodar_casos EXCLUDE_FROM_ALL testes/rodar_casos.cpp ${GALS_SOURCES} ${SEMANTICO_SOURCES} ${TOKENS_SOURCES})
target_include_directories(rodar_casos PRIVATE ${CMAKE_SOURCE_DIR})
add_custom_target(testar
    COMMAND rodar_casos "${CMAKE_SOURCE_DIR}/casos_de_teste.txt"
    DEPENDS rodar_casos
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    USES_TERMINAL
)
