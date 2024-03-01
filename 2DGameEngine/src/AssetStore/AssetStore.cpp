#include "AssetStore.h"
#include "../Logger/Logger.h"
#include <SDL_image.h>

AssetStore::AssetStore() {
	Logger::Log("AssetStore constructor called");
}

AssetStore::~AssetStore() {
	ClearAssets();
	Logger::Log("AssetStore deconstructor called");
}

void AssetStore::ClearAssets() {
	for (auto texture : textures) {
		SDL_DestroyTexture(texture.second);
	}
	textures.clear();
}

void AssetStore::AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath) {
	SDL_Surface* surface = IMG_Load(filePath.c_str());
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	// Add the texture to the map
	textures.emplace(assetId, texture);
}

SDL_Texture* AssetStore::GetTexture(const std::string& assetId) {
	return textures[assetId];
}