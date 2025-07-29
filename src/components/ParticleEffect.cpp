#include "Components.h"
#include <algorithm>
#include <random>
#include <cmath>

void ParticleEffect::update(float deltaTime, const Vector2& emitterPosition) {
    systemTime += deltaTime;
    emissionTimer += deltaTime;
    
    // Emit new particles if emitting
    if (isEmitting && continuous) {
        float emissionInterval = 1.0f / emissionRate;
        while (emissionTimer >= emissionInterval && getActiveParticleCount() < maxParticles) {
            emitParticles(1);
            emissionTimer -= emissionInterval;
        }
    }
    
    // Update existing particles
    for (auto& particle : particles) {
        if (!particle.active || particle.life <= 0.0f) continue;
        
        // Update physics
        particle.velocity = particle.velocity + (particle.acceleration + gravity) * deltaTime;
        particle.position = particle.position + particle.velocity * deltaTime;
        
        // Update rotation
        particle.rotation += particle.rotationSpeed * deltaTime;
        
        // Update life
        particle.life -= deltaTime;
        
        // Update visual properties based on life
        float lifeRatio = particle.life / particle.maxLife;
        particle.color = interpolateColor(endColor, startColor, lifeRatio);
        
        // Update size over lifetime
        if (sizeOverLife != 1.0f) {
            float sizeMultiplier = 1.0f + (sizeOverLife - 1.0f) * (1.0f - lifeRatio);
            particle.size = particle.size * sizeMultiplier;
        }
        
        // Deactivate dead particles
        if (particle.life <= 0.0f) {
            particle.active = false;
        }
    }
    
    // Remove dead particles (keep vector size manageable)
    if (particles.size() > maxParticles * 2) {
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [](const Particle& p) { return !p.active; }),
            particles.end()
        );
    }
}

void ParticleEffect::emitParticles(int count) {
    for (int i = 0; i < count && getActiveParticleCount() < maxParticles; ++i) {
        Particle particle;
        
        // Set position based on emission shape
        particle.position = getRandomPosition();
        
        // Set velocity
        particle.velocity = getRandomVelocity();
        
        // Set lifetime
        particle.life = particle.maxLife = randomFloat(minLifetime, maxLifetime);
        
        // Set size
        particle.size = randomFloat(minSize, maxSize);
        
        // Set rotation
        particle.rotation = randomFloat(minRotation, maxRotation);
        particle.rotationSpeed = randomFloat(minRotationSpeed, maxRotationSpeed);
        
        // Set color (start with start color)
        particle.color = startColor;
        
        particle.active = true;
        
        particles.push_back(particle);
    }
}

Vector2 ParticleEffect::getRandomVelocity() const {
    Vector2 vel;
    
    switch (shape) {
        case EmissionShape::Cone: {
            // Generate velocity in cone direction
            float angle = atan2(direction.y, direction.x);
            float coneRad = (coneAngle * 3.14159f / 180.0f) * 0.5f;
            float randomAngle = angle + randomFloat(-coneRad, coneRad);
            
            float speed = randomFloat(
                sqrt(minVelocity.x * minVelocity.x + minVelocity.y * minVelocity.y),
                sqrt(maxVelocity.x * maxVelocity.x + maxVelocity.y * maxVelocity.y)
            );
            
            vel.x = cos(randomAngle) * speed;
            vel.y = sin(randomAngle) * speed;
            break;
        }
        default:
            // For other shapes, use random velocity in range
            vel.x = randomFloat(minVelocity.x, maxVelocity.x);
            vel.y = randomFloat(minVelocity.y, maxVelocity.y);
            break;
    }
    
    return vel;
}

Vector2 ParticleEffect::getRandomPosition() const {
    Vector2 pos(0, 0);
    
    switch (shape) {
        case EmissionShape::Point:
            // No offset from center
            break;
            
        case EmissionShape::Box:
            pos.x = randomFloat(-emissionSize.x * 0.5f, emissionSize.x * 0.5f);
            pos.y = randomFloat(-emissionSize.y * 0.5f, emissionSize.y * 0.5f);
            break;
            
        case EmissionShape::Circle: {
            float angle = randomFloat(0.0f, 2.0f * 3.14159f);
            float radius = randomFloat(0.0f, emissionSize.x * 0.5f);
            pos.x = cos(angle) * radius;
            pos.y = sin(angle) * radius;
            break;
        }
        
        case EmissionShape::Cone:
            // For cone, emit from point but in cone direction
            break;
    }
    
    return pos;
}

Color ParticleEffect::interpolateColor(const Color& start, const Color& end, float t) const {
    t = std::max(0.0f, std::min(1.0f, t)); // Clamp t to [0,1]
    
    return Color(
        static_cast<uint8_t>(start.r + (end.r - start.r) * t),
        static_cast<uint8_t>(start.g + (end.g - start.g) * t),
        static_cast<uint8_t>(start.b + (end.b - start.b) * t),
        static_cast<uint8_t>(start.a + (end.a - start.a) * t)
    );
}

float ParticleEffect::randomFloat(float min, float max) const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}
