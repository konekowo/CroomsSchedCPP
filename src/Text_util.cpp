#include <string>
#include <unordered_map>
#include <SDL3_ttf/SDL_ttf.h>

static SDL_Renderer* renderer = nullptr;
static std::pmr::unordered_map<std::string, SDL_Texture*> textureMap;

void TextUtil_init(SDL_Renderer* _renderer) {
    renderer = _renderer;
}

SDL_FRect RenderText(TTF_Font* font, const char* textkey, const char* text, const float x, const float y, const SDL_Color color, const float scale) {
    SDL_Texture* texture = textureMap[textkey];
    bool newTexture = false;
    if (texture == nullptr) {
        SDL_Surface* surface = TTF_RenderText_Blended(font, text, 0, color);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
        newTexture = true;
    }
    const SDL_FRect dstRect = {x, y, static_cast<float>(texture->w) * scale, static_cast<float>(texture->h) * scale}; // x, y, w, h
    SDL_RenderTexture(renderer, texture, nullptr, &dstRect);
    if (newTexture) {
        textureMap[textkey] = texture;
    }
    return dstRect;
}

void DestroyText(const char* textKey) {
    SDL_Texture* texture = textureMap[textKey];
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
        textureMap[textKey] = nullptr;
        textureMap.erase(textKey);
    }
}
