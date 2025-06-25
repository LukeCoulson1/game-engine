"""
Core game engine module.
Contains the main game loop, rendering, and system management.
"""

import pygame
import sys
from typing import Dict, List, Optional
from engine.scene_manager import SceneManager
from engine.input_manager import InputManager
from engine.renderer import Renderer
from engine.audio_manager import AudioManager
from engine.resource_manager import ResourceManager


class GameEngine:
    """Main game engine class that manages all systems."""
    
    def __init__(self, width: int = 1024, height: int = 768, title: str = "2D Game Engine"):
        """Initialize the game engine."""
        pygame.init()
        
        self.width = width
        self.height = height
        self.title = title
        self.running = False
        self.clock = pygame.time.Clock()
        self.target_fps = 60
        
        # Initialize display
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption(title)
        
        # Initialize engine systems
        self.resource_manager = ResourceManager()
        self.renderer = Renderer(self.screen)
        self.scene_manager = SceneManager()
        self.input_manager = InputManager()
        self.audio_manager = AudioManager()
        
        # Engine state
        self.delta_time = 0.0
        self.total_time = 0.0
        
    def run(self):
        """Main game loop."""
        self.running = True
        last_time = pygame.time.get_ticks()
        
        while self.running:
            # Calculate delta time
            current_time = pygame.time.get_ticks()
            self.delta_time = (current_time - last_time) / 1000.0
            self.total_time += self.delta_time
            last_time = current_time
            
            # Handle events
            self._handle_events()
            
            # Update systems
            self._update()
            
            # Render
            self._render()
            
            # Cap framerate
            self.clock.tick(self.target_fps)
            
        self._cleanup()
    
    def _handle_events(self):
        """Handle pygame events."""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            else:
                self.input_manager.handle_event(event)
    
    def _update(self):
        """Update all game systems."""
        self.input_manager.update()
        self.scene_manager.update(self.delta_time)
        self.audio_manager.update()
    
    def _render(self):
        """Render the current frame."""
        self.renderer.clear()
        self.scene_manager.render(self.renderer)
        self.renderer.present()
    
    def _cleanup(self):
        """Clean up resources before shutting down."""
        self.audio_manager.cleanup()
        pygame.quit()
        sys.exit()
    
    def stop(self):
        """Stop the game engine."""
        self.running = False
    
    def get_screen_size(self):
        """Get the screen dimensions."""
        return (self.width, self.height)
    
    def set_target_fps(self, fps: int):
        """Set the target framerate."""
        self.target_fps = fps
