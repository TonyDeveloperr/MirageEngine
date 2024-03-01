#pragma once
#include <SDL.h>
#include "../AssetStore/AssetStore.h"
class RenderSystem: public System {

public:
	RenderSystem() {
		RequireComponent<TransformComponent>();
		RequireComponent<SpriteComponent>();
	}

	void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore) {

		for (auto entity : GetSystemEntities()) {
			const auto transform = entity.GetComponent<TransformComponent>();
			const auto sprite = entity.GetComponent<SpriteComponent>();
			SDL_Texture* texture = assetStore->GetTexture(sprite.assetId);

			// Set the source rectangle for our original sprite texture
			SDL_Rect srcRect = sprite.srcRect;

			if (srcRect.w == 0 && srcRect.h == 0) {
				SDL_QueryTexture(texture, NULL, NULL, &srcRect.w, &srcRect.h);
			}
			auto spriteWidth = sprite.width;
			auto spriteHeight = sprite.height;

			if (spriteWidth == 0 && spriteHeight == 0) {
				spriteWidth = srcRect.w;
				spriteHeight = srcRect.h;
			}

			// Set the destination rectangle with the x, y position to be rendered
			SDL_Rect dstRect = {
				static_cast<int>(transform.position.x),
				static_cast<int>(transform.position.y),
				static_cast<int>(spriteWidth * transform.scale.x),
				static_cast<int>(spriteWidth * transform.scale.y)
			};

			// Draw PNG texture based on the sprite ID
			SDL_RenderCopyEx(
				renderer,
				texture,
				&srcRect,
				&dstRect,
				transform.rotation,
				NULL,
				SDL_FLIP_NONE
			);
				


		}
	}

};
