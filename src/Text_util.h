#ifndef TEXT_UTIL_H
#define TEXT_UTIL_H
#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>

void TextUtil_init(SDL_Renderer* _renderer);
SDL_FRect RenderText(TTF_Font* font, const char* textkey, const char* text, float x, float y, SDL_Color color, float scale);
void DestroyText(const char* textKey);

#endif //TEXT_UTIL_H
