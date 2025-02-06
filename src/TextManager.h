#pragma once
#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <unordered_map>

struct TextureData {
    SDL_Texture* texture;
    std::string text;
    SDL_Color color;
    TTF_Font* font;
};

class TextManager {
    SDL_Renderer *renderer;
    TTF_Font *font = nullptr;
    std::pmr::unordered_map<std::string, TextureData*> textureMap;
public:
    explicit TextManager(SDL_Renderer* renderer);
    SDL_FRect RenderText(TTF_Font* font, const std::string& textKey, const std::string& text, float x, float y, SDL_Color color, float scale);
    void DestroyText(const std::string& textKey);
};