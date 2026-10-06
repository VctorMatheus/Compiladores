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
    Semantico.cpp Semantico.h
    Token.h
    AnalysisError.h LexicalError.h SyntacticError.h SemanticError.h
)

# Arquivos da IDE
set(IDE_SOURCES
    main.cpp
    MainWindow.cpp MainWindow.h
    Compilador.cpp Compilador.h
)

add_executable(CompiladorIDE ${IDE_SOURCES} ${GALS_SOURCES})

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
