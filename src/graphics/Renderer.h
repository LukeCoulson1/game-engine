#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <memory>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

struct Color {
    uint8_t r, g, b, a;
    
    Color(uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

struct Vector2 {
    float x, y;
    
    Vector2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
    
    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
};

struct Rect {
    float x, y, width, height;
    
    Rect(float x = 0.0f, float y = 0.0f, float width = 0.0f, float height = 0.0f)
        : x(x), y(y), width(width), height(height) {}
};

class Texture {
public:
    Texture(SDL_Texture* texture, int width, int height, const std::string& filepath = "");
    ~Texture();
    
    SDL_Texture* getSDLTexture() const { return m_texture; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const std::string& getFilePath() const { return m_filepath; }

private:
    SDL_Texture* m_texture;
    int m_width, m_height;
    std::string m_filepath; // Store the original file path for serialization
};

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    bool initialize(const std::string& title, int width, int height);
    void shutdown();
    
    void clear(const Color& color = Color(0, 0, 0, 255));
    void present();
    
    // Drawing functions
    void drawRect(const Rect& rect, const Color& color, bool filled = true);
    void drawTexture(std::shared_ptr<Texture> texture, const Vector2& position);
    void drawTexture(std::shared_ptr<Texture> texture, const Rect& srcRect, const Rect& dstRect);
    void drawTexture(std::shared_ptr<Texture> texture, const Rect& srcRect, const Rect& dstRect, 
                    float rotation, const Vector2& center = Vector2(0, 0));
    
    // Texture creation
    std::shared_ptr<Texture> loadTexture(const std::string& filepath);
    std::shared_ptr<Texture> createTexture(int width, int height);
    
    // Camera
    void setCamera(const Vector2& position) { m_cameraPosition = position; }
    Vector2 getCamera() const { return m_cameraPosition; }
      // Screen to world conversion
    Vector2 screenToWorld(const Vector2& screenPos) const;
    Vector2 worldToScreen(const Vector2& worldPos) const;
    
    SDL_Renderer* getSDLRenderer() const { return m_renderer; }
    SDL_Window* getWindow() const { return m_window; }

private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    Vector2 m_cameraPosition;
    int m_windowWidth, m_windowHeight;
};
