#include "Text_util.h"

#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <unordered_map>


TextManager::TextManager(SDL_Renderer *renderer) { this->renderer = renderer; }
SDL_FRect TextManager::RenderText(TTF_Font *font, const std::string &textKey, const std::string &text, const float x,
                                  const float y, const SDL_Color color, const float scale) {
    TextureData *data = textureMap[textKey];
    bool newTexture = false;
    if (data == nullptr || data->texture == nullptr || data->text != text || data->color.r != color.r ||
        data->color.g != color.g || data->color.b != color.b) {
        if (data != nullptr) {
            DestroyText(textKey);
        }
        SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
        data = new TextureData{
                .texture = SDL_CreateTextureFromSurface(renderer, surface), .text = text, .color = color, .font = font};
        SDL_DestroySurface(surface);
        newTexture = true;
    }
    const SDL_FRect dstRect = {x, y, static_cast<float>(data->texture->w) * scale,
                               static_cast<float>(data->texture->h) * scale};
    SDL_RenderTexture(renderer, data->texture, nullptr, &dstRect);
    if (newTexture) {
        textureMap[textKey] = data;
    }
    return dstRect;
}
void TextManager::DestroyText(const std::string &textKey) {
    if (const TextureData *data = textureMap[textKey]; data != nullptr) {
        if (data->texture != nullptr) {
            SDL_DestroyTexture(data->texture);
        }
        textureMap[textKey] = nullptr;
        textureMap.erase(textKey);
        delete data;
    }
}
