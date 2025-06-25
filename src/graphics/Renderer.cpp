#include "Renderer.h"
#include <SDL2/SDL_image.h>
#include <iostream>

// Texture Implementation
Texture::Texture(SDL_Texture* texture, int width, int height)
    : m_texture(texture), m_width(width), m_height(height) {
}

Texture::~Texture() {
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
    }
}

// Renderer Implementation
Renderer::Renderer() : m_window(nullptr), m_renderer(nullptr), m_cameraPosition(0, 0) {
}

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::initialize(const std::string& title, int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
    
    // Create window
    m_window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    
    if (!m_window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create renderer
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }
    
    return true;
}

void Renderer::shutdown() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    
    IMG_Quit();
}

void Renderer::clear(const Color& color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(m_renderer);
}

void Renderer::present() {
    SDL_RenderPresent(m_renderer);
}

void Renderer::drawRect(const Rect& rect, const Color& color, bool filled) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    
    // Apply camera transformation
    SDL_Rect sdlRect = {
        static_cast<int>(rect.x - m_cameraPosition.x),
        static_cast<int>(rect.y - m_cameraPosition.y),
        static_cast<int>(rect.width),
        static_cast<int>(rect.height)
    };
    
    if (filled) {
        SDL_RenderFillRect(m_renderer, &sdlRect);
    } else {
        SDL_RenderDrawRect(m_renderer, &sdlRect);
    }
}

void Renderer::drawTexture(std::shared_ptr<Texture> texture, const Vector2& position) {
    if (!texture) return;
    
    SDL_Rect dstRect = {
        static_cast<int>(position.x - m_cameraPosition.x),
        static_cast<int>(position.y - m_cameraPosition.y),
        texture->getWidth(),
        texture->getHeight()
    };
    
    SDL_RenderCopy(m_renderer, texture->getSDLTexture(), nullptr, &dstRect);
}

void Renderer::drawTexture(std::shared_ptr<Texture> texture, const Rect& srcRect, const Rect& dstRect) {
    if (!texture) return;
    
    SDL_Rect src = {
        static_cast<int>(srcRect.x),
        static_cast<int>(srcRect.y),
        static_cast<int>(srcRect.width),
        static_cast<int>(srcRect.height)
    };
    
    SDL_Rect dst = {
        static_cast<int>(dstRect.x - m_cameraPosition.x),
        static_cast<int>(dstRect.y - m_cameraPosition.y),
        static_cast<int>(dstRect.width),
        static_cast<int>(dstRect.height)
    };
    
    SDL_RenderCopy(m_renderer, texture->getSDLTexture(), &src, &dst);
}

void Renderer::drawTexture(std::shared_ptr<Texture> texture, const Rect& srcRect, const Rect& dstRect, 
                          float rotation, const Vector2& center) {
    if (!texture) return;
    
    SDL_Rect src = {
        static_cast<int>(srcRect.x),
        static_cast<int>(srcRect.y),
        static_cast<int>(srcRect.width),
        static_cast<int>(srcRect.height)
    };
    
    SDL_Rect dst = {
        static_cast<int>(dstRect.x - m_cameraPosition.x),
        static_cast<int>(dstRect.y - m_cameraPosition.y),
        static_cast<int>(dstRect.width),
        static_cast<int>(dstRect.height)
    };
      // Center point should be relative to the destination rectangle (not world space)
    SDL_Point centerPoint = {
        static_cast<int>(center.x),
        static_cast<int>(center.y)
    };
    
    // Debug output for scaled/rotated sprites
    if (rotation != 0.0f || dst.w != src.w || dst.h != src.h) {
        printf("SDL_RenderCopyEx DEBUG:\n");
        printf("  src: %d,%d,%d,%d\n", src.x, src.y, src.w, src.h);
        printf("  dst: %d,%d,%d,%d\n", dst.x, dst.y, dst.w, dst.h);
        printf("  rotation: %.1f\n", rotation);
        printf("  center: %d,%d\n", centerPoint.x, centerPoint.y);
        fflush(stdout);
    }
    
    SDL_RenderCopyEx(m_renderer, texture->getSDLTexture(), &src, &dst, 
                     rotation, &centerPoint, SDL_FLIP_NONE);
}

std::shared_ptr<Texture> Renderer::loadTexture(const std::string& filepath) {
    SDL_Surface* surface = IMG_Load(filepath.c_str());
    if (!surface) {
        std::cerr << "Unable to load image " << filepath << "! SDL_image Error: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (!texture) {
        std::cerr << "Unable to create texture from " << filepath << "! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return nullptr;
    }
    
    int width = surface->w;
    int height = surface->h;
    SDL_FreeSurface(surface);
    
    return std::make_shared<Texture>(texture, width, height);
}

std::shared_ptr<Texture> Renderer::createTexture(int width, int height) {
    SDL_Texture* texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, 
                                           SDL_TEXTUREACCESS_TARGET, width, height);
    if (!texture) {
        std::cerr << "Unable to create texture! SDL Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    return std::make_shared<Texture>(texture, width, height);
}

Vector2 Renderer::screenToWorld(const Vector2& screenPos) const {
    return Vector2(screenPos.x + m_cameraPosition.x, screenPos.y + m_cameraPosition.y);
}

Vector2 Renderer::worldToScreen(const Vector2& worldPos) const {
    return Vector2(worldPos.x - m_cameraPosition.x, worldPos.y - m_cameraPosition.y);
}
