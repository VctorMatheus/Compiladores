#ifndef CATALOGOTOKENS_H
#define CATALOGOTOKENS_H

#include "Constants.h"

#include <vector>

// Todos os tokens da Roman++ (os da seção #Tokens do Roman++.vgls), na ordem do enum TokenId,
// seguidos do que o léxico descarta (espaços e comentários). Mostrado na aba Tokens da IDE.
// O rodar_casos confere o catálogo com o léxico e o sintático gerados: ao mexer nos tokens do
// .vgls, atualize o CatalogoTokens.cpp e rode o "testar".
struct InfoToken
{
    TokenId id;            // EPSILON nos itens descartados, que nunca viram token
    const char *nome;      // nome no .vgls (IF, SUM...); nullptr nos descartados
    const char *lexema;    // lexema fixo ou, se houver exemplo, a descrição do padrão
    const char *exemplo;   // lexema que casa com o padrão; nullptr quando o lexema é fixo
    const char *categoria;
    const char *significado;
    bool semUso = false;   // o léxico reconhece, mas nenhuma regra da gramática aceita
};

const std::vector<InfoToken> &catalogoTokens();

#endif
